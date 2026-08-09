#include "playback_session.h"

#include "backend/playback_session_backend.h"
#include "playback/domain/errors/playback_failure.h"
#include "playback/domain/state/playback_invariants.h"
#include "playback/domain/state/playback_reducer.h"

#include <QThread>
#include <QTimer>

#include <chrono>
#include <limits>
#include <utility>
#include <variant>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;

constexpr std::chrono::milliseconds kRequestTimeout{30000};
constexpr int kRequestTimeoutPollMilliseconds = 1000;

PlaybackFailure makeFailure(PlaybackFailureCategory category, QString diagnostic)
{
    if (diagnostic.isEmpty()) {
        diagnostic = QStringLiteral("Playback session operation failed.");
    }
    return PlaybackFailure{category, 0, std::move(diagnostic)};
}

QString requestTrackDiagnostic(RequestTrackStatus status)
{
    switch (status) {
    case RequestTrackStatus::Tracked:
        return {};
    case RequestTrackStatus::InvalidRequestId:
        return QStringLiteral("RequestTracker rejected an invalid request id.");
    case RequestTrackStatus::DuplicateRequestId:
        return QStringLiteral("RequestTracker rejected a duplicate request id.");
    case RequestTrackStatus::NotTrackable:
        return QStringLiteral("RequestTracker rejected a command without an asynchronous backend request.");
    }
    return QStringLiteral("RequestTracker rejected a playback request.");
}

bool isMediaPropertyLifecycle(PlaybackLifecycleState lifecycle) noexcept
{
    return lifecycle == PlaybackLifecycleState::Opening
        || lifecycle == PlaybackLifecycleState::Ready
        || lifecycle == PlaybackLifecycleState::Ended;
}

bool shouldApplyBackendEvent(
    const PlaybackSnapshot& snapshot,
    const PlaybackEvent& event) noexcept
{
    const PlaybackLifecycleState lifecycle = snapshot.lifecycle();
    const PlaybackEventPayload& payload = event.payload;

    if (std::holds_alternative<PauseChangedEvent>(payload)
        || std::holds_alternative<SeekingChangedEvent>(payload)) {
        return lifecycle == PlaybackLifecycleState::Ready;
    }

    if (std::holds_alternative<BufferingChangedEvent>(payload)
        || std::holds_alternative<BufferingProgressChangedEvent>(payload)) {
        return lifecycle == PlaybackLifecycleState::Opening
            || lifecycle == PlaybackLifecycleState::Ready;
    }

    if (std::holds_alternative<PositionChangedEvent>(payload)
        || std::holds_alternative<DurationChangedEvent>(payload)
        || std::holds_alternative<SeekableChangedEvent>(payload)
        || std::holds_alternative<MediaTitleChangedEvent>(payload)
        || std::holds_alternative<MediaPathChangedEvent>(payload)) {
        return isMediaPropertyLifecycle(lifecycle);
    }

    return true;
}

} // namespace

PlaybackSession::PlaybackSession(QObject* parent)
    : QObject(parent)
    , backend_(std::make_unique<PlaybackSessionBackend>())
{
}

PlaybackSession::~PlaybackSession()
{
    backend_->shutdown();
}

void PlaybackSession::initialize()
{
    if (!isOnOwningThread()) {
        emit startupFailed(QStringLiteral("PlaybackSession must initialize on its owning playback thread."));
        return;
    }
    if (initialized_) {
        return;
    }

    stopping_ = false;
    backend_->setEventHandler([this](const PlaybackEvent& event) {
        handleBackendEvent(event);
    });

    QString error;
    if (!backend_->initialize(&error)) {
        backend_->setEventHandler({});
        emit startupFailed(
            error.isEmpty() ? QStringLiteral("PlaybackSession backend initialization failed.") : error);
        return;
    }

    ensureRequestTimeoutTimer();
    requestTimeoutTimer_->start();
    initialized_ = true;
    emit ready();
}

void PlaybackSession::shutdown()
{
    if (!isOnOwningThread()) {
        return;
    }
    if (stopping_) {
        return;
    }

    stopping_ = true;
    if (requestTimeoutTimer_ != nullptr) {
        requestTimeoutTimer_->stop();
    }
    (void)requestTracker_.cancelAll(PlaybackRequestCancellationReason::Shutdown);
    backend_->setEventHandler({});
    backend_->shutdown();

    if (initialized_) {
        commitSnapshot(reducePlaybackSnapshot(
            snapshot_,
            PlaybackEvent{PlaybackBackendShutdownEvent{}}));
    }

    initialized_ = false;
    emit stopped();
}

void PlaybackSession::processCommand(PlaybackCommand command)
{
    if (!isOnOwningThread() || stopping_) {
        return;
    }

    if (const auto validationError = validatePlaybackCommand(command); validationError.has_value()) {
        commitSnapshot(reducePlaybackSnapshot(
            snapshot_,
            PlaybackEvent{PlaybackFailureEvent{makeFailure(
                PlaybackFailureCategory::Protocol,
                QStringLiteral("PlaybackSession rejected an invalid domain command."))}}));
        return;
    }

    if (const auto* lifecycle = std::get_if<LifecycleCommand>(&command.payload())) {
        if (lifecycle->action == PlaybackLifecycleAction::Initialize) {
            initialize();
        } else {
            shutdown();
        }
        return;
    }

    if (!initialized_ || !backend_->isReady()) {
        commitSubmissionFailure(
            command,
            QStringLiteral("PlaybackSession is not initialized."));
        return;
    }

    if (std::holds_alternative<LoadMediaCommand>(command.payload())) {
        beginMediaLoad(command);
        return;
    }

    submitTrackedCommand(command);
}

void PlaybackSession::handleBackendEvent(const PlaybackEvent& event)
{
    if (!isOnOwningThread() || !initialized_ || stopping_) {
        return;
    }

    if (const auto* reply = std::get_if<CommandReplyEvent>(&event.payload)) {
        handleCommandReply(*reply);
        return;
    }

    if (!shouldApplyBackendEvent(snapshot_, event)) {
        return;
    }

    commitSnapshot(reducePlaybackSnapshot(snapshot_, event));
}

void PlaybackSession::handleCommandReply(const CommandReplyEvent& reply)
{
    const RequestReplyResolution resolution = requestTracker_.resolve(
        reply,
        snapshot_.generation());
    if (!resolution.accepted() || reply.succeeded) {
        return;
    }

    PlaybackFailure failure = reply.failure.has_value()
        ? *reply.failure
        : makeFailure(
            PlaybackFailureCategory::Command,
            QStringLiteral("Playback backend returned a failed command reply without diagnostics."));

    if (resolution.record.has_value()
        && resolution.record->type == PlaybackRequestType::LoadMedia) {
        commitSnapshot(reducePlaybackSnapshot(
            snapshot_,
            PlaybackEvent{MediaFailedEvent{std::move(failure)}}));
        return;
    }

    commitSnapshot(reducePlaybackSnapshot(
        snapshot_,
        PlaybackEvent{PlaybackFailureEvent{std::move(failure)}}));
}

void PlaybackSession::beginMediaLoad(const PlaybackCommand& command)
{
    const auto* load = std::get_if<LoadMediaCommand>(&command.payload());
    if (load == nullptr) {
        return;
    }

    const MediaGeneration generation = allocateMediaGeneration();
    if (!generation.isValid()) {
        commitSubmissionFailure(
            command,
            QStringLiteral("Playback media generation space is exhausted."));
        return;
    }

    const RequestTrackStatus trackStatus = requestTracker_.track(command, generation);
    if (trackStatus != RequestTrackStatus::Tracked) {
        commitTrackingFailure(trackStatus);
        return;
    }

    (void)requestTracker_.cancelMediaRequestsForGenerationChange(generation);

    PlaybackSnapshotState seededState = snapshot_.state();
    seededState.generation = generation;
    seededState.media.source = load->source;

    const PlaybackSnapshot opening = reducePlaybackSnapshot(
        PlaybackSnapshot{std::move(seededState)},
        PlaybackEvent{MediaLoadStartedEvent{}});
    commitSnapshot(opening);

    QString error;
    if (!backend_->submit(command, &error)) {
        (void)requestTracker_.cancel(
            command.requestId(),
            PlaybackRequestCancellationReason::SubmissionFailed);
        commitSubmissionFailure(command, std::move(error));
    }
}

void PlaybackSession::submitTrackedCommand(const PlaybackCommand& command)
{
    const RequestTrackStatus trackStatus = requestTracker_.track(
        command,
        snapshot_.generation());
    if (trackStatus != RequestTrackStatus::Tracked) {
        commitTrackingFailure(trackStatus);
        return;
    }

    QString error;
    if (!backend_->submit(command, &error)) {
        (void)requestTracker_.cancel(
            command.requestId(),
            PlaybackRequestCancellationReason::SubmissionFailed);
        commitSubmissionFailure(command, std::move(error));
    }
}

void PlaybackSession::commitSnapshot(PlaybackSnapshot next)
{
    const PlaybackInvariantViolations snapshotViolations = checkPlaybackSnapshotInvariants(next);
    const PlaybackInvariantViolations transitionViolations = checkPlaybackTransitionInvariants(
        snapshot_,
        next);
    const int violationCount = static_cast<int>(
        snapshotViolations.size() + transitionViolations.size());

    if (violationCount > 0) {
        emit invariantViolationDetected(violationCount);
    }

    snapshot_ = std::move(next);
    emit snapshotCommitted(snapshot_);
}

void PlaybackSession::commitSubmissionFailure(
    const PlaybackCommand& command,
    QString diagnostic)
{
    if (std::holds_alternative<LoadMediaCommand>(command.payload())) {
        commitSnapshot(reducePlaybackSnapshot(
            snapshot_,
            PlaybackEvent{MediaFailedEvent{makeFailure(
                PlaybackFailureCategory::Media,
                std::move(diagnostic))}}));
        return;
    }

    commitSnapshot(reducePlaybackSnapshot(
        snapshot_,
        PlaybackEvent{PlaybackFailureEvent{makeFailure(
            PlaybackFailureCategory::Command,
            std::move(diagnostic))}}));
}

void PlaybackSession::commitTrackingFailure(RequestTrackStatus status)
{
    commitSnapshot(reducePlaybackSnapshot(
        snapshot_,
        PlaybackEvent{PlaybackFailureEvent{makeFailure(
            PlaybackFailureCategory::Protocol,
            requestTrackDiagnostic(status))}}));
}

void PlaybackSession::ensureRequestTimeoutTimer()
{
    if (requestTimeoutTimer_ != nullptr) {
        return;
    }

    requestTimeoutTimer_ = new QTimer(this);
    requestTimeoutTimer_->setInterval(kRequestTimeoutPollMilliseconds);
    requestTimeoutTimer_->setTimerType(Qt::CoarseTimer);
    QObject::connect(
        requestTimeoutTimer_,
        &QTimer::timeout,
        this,
        [this]() {
            (void)requestTracker_.cancelExpired(
                PlaybackRequestClock::now(),
                kRequestTimeout);
        });
}

bool PlaybackSession::isOnOwningThread() const noexcept
{
    return QThread::currentThread() == thread();
}

MediaGeneration PlaybackSession::allocateMediaGeneration() noexcept
{
    if (nextGenerationValue_ == 0) {
        return {};
    }

    const quint64 value = nextGenerationValue_;
    if (nextGenerationValue_ == std::numeric_limits<quint64>::max()) {
        nextGenerationValue_ = 0;
    } else {
        ++nextGenerationValue_;
    }
    return MediaGeneration{value};
}

} // namespace player::playback::application
