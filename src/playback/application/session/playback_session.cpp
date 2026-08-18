#include "playback_session.h"

#include "backend/playback_session_backend.h"
#include "playback_shutdown.h"
#include "playback/application/requests/request_timeout_monitor.h"
#include "playback/domain/errors/playback_failure.h"
#include "playback/domain/state/playback_invariants.h"
#include "playback/domain/state/playback_reducer.h"

#include <QThread>
#include <QtGlobal>

#include <limits>
#include <utility>
#include <variant>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;

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
        || std::holds_alternative<BufferingProgressChangedEvent>(payload)
        || std::holds_alternative<CacheStatusChangedEvent>(payload)) {
        return lifecycle == PlaybackLifecycleState::Opening
            || lifecycle == PlaybackLifecycleState::Ready;
    }

    if (std::holds_alternative<PositionChangedEvent>(payload)
        || std::holds_alternative<DurationChangedEvent>(payload)
        || std::holds_alternative<SeekableChangedEvent>(payload)
        || std::holds_alternative<MediaTitleChangedEvent>(payload)
        || std::holds_alternative<MediaPathChangedEvent>(payload)
        || std::holds_alternative<TrackListChangedEvent>(payload)
        || std::holds_alternative<SelectedVideoTrackChangedEvent>(payload)
        || std::holds_alternative<SelectedAudioTrackChangedEvent>(payload)
        || std::holds_alternative<SelectedSubtitleTrackChangedEvent>(payload)
        || std::holds_alternative<ChapterListChangedEvent>(payload)
        || std::holds_alternative<VideoStreamInfoChangedEvent>(payload)
        || std::holds_alternative<AudioStreamInfoChangedEvent>(payload)
        || std::holds_alternative<SubtitleDelayChangedEvent>(payload)
        || std::holds_alternative<AudioDelayChangedEvent>(payload)) {
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
    if (!isOnOwningThread()) {
        if (initialized_) {
            qFatal("PlaybackSession with live backend resources must be destroyed on its owning playback thread.");
        }
        return;
    }

    executePlaybackShutdown(requestTracker_, requestTimeoutMonitor_.get(), *backend_);
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
    mediaGenerationGate_.reset();
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

    if (requestTimeoutMonitor_ == nullptr) {
        requestTimeoutMonitor_ = std::make_unique<RequestTimeoutMonitor>(
            requestTracker_,
            [this](const PlaybackRequestRecord& record) {
                handleRequestTimeout(record);
            });
    }
    requestTimeoutMonitor_->start();
    initialized_ = true;
    emit renderCoreReady(backend_->renderCoreAddress());
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
    executePlaybackShutdown(requestTracker_, requestTimeoutMonitor_.get(), *backend_);

    if (initialized_) {
        commitSnapshot(reducePlaybackSnapshot(
            snapshot_,
            PlaybackEvent{PlaybackBackendShutdownEvent{}}));
    }

    mediaGenerationGate_.reset();
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

    if (!mediaGenerationGate_.accepts(event)) {
        return;
    }

    if (!shouldApplyBackendEvent(snapshot_, event)) {
        return;
    }

    const auto* ended = std::get_if<MediaEndedEvent>(&event.payload);
    commitSnapshot(reducePlaybackSnapshot(snapshot_, event));
    if (ended != nullptr
        && (ended->reason == MediaEndReason::Stopped
            || ended->reason == MediaEndReason::Shutdown)) {
        (void)requestTracker_.cancelMediaRequestsForGeneration(
            event.generation,
            PlaybackRequestCancellationReason::GenerationChanged);
        mediaGenerationGate_.reset();
    }
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

    if (resolution.record.has_value()) {
        emit requestFailed(
            static_cast<quint8>(resolution.record->type),
            failure.diagnostic);
    }

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

void PlaybackSession::handleRequestTimeout(const PlaybackRequestRecord& record)
{
    if (!initialized_ || stopping_
        || record.type != PlaybackRequestType::LoadMedia
        || !record.generation.has_value()
        || *record.generation != snapshot_.generation()
        || snapshot_.lifecycle() != PlaybackLifecycleState::Opening) {
        return;
    }

    const QString diagnostic = QStringLiteral("Media load timed out.");
    emit requestFailed(static_cast<quint8>(record.type), diagnostic);

    // The timed-out generation must no longer be allowed to mutate state. A
    // late backend reply/event remains observable by request diagnostics but
    // cannot resurrect a load that the product has already declared failed.
    mediaGenerationGate_.reset();
    commitSnapshot(reducePlaybackSnapshot(
        snapshot_,
        PlaybackEvent{MediaFailedEvent{makeFailure(
            PlaybackFailureCategory::Media,
            diagnostic)}}));
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

    (void)requestTracker_.supersedePendingFor(command, generation);
    (void)requestTracker_.cancelMediaRequestsForGenerationChange(generation);
    mediaGenerationGate_.activate(generation);

    PlaybackSnapshotState seededState = snapshot_.state();
    seededState.generation = generation;
    seededState.media.source = load->source;

    const PlaybackSnapshot opening = reducePlaybackSnapshot(
        PlaybackSnapshot{std::move(seededState)},
        PlaybackEvent{MediaLoadStartedEvent{}});
    commitSnapshot(opening);

    const std::optional<PlaybackRequestRecord> trackedRecord = requestTracker_.record(
        command.requestId());
    QString error;
    if (!backend_->submit(command, generation, &error)) {
        (void)requestTracker_.cancel(
            command.requestId(),
            PlaybackRequestCancellationReason::SubmissionFailed);
        if (trackedRecord.has_value()) {
            emit requestFailed(
                static_cast<quint8>(trackedRecord->type),
                error);
        }
        commitSubmissionFailure(command, std::move(error));
    }
}

void PlaybackSession::submitTrackedCommand(const PlaybackCommand& command)
{
    const MediaGeneration generation = snapshot_.generation();
    const RequestTrackStatus trackStatus = requestTracker_.track(
        command,
        generation);
    if (trackStatus != RequestTrackStatus::Tracked) {
        commitTrackingFailure(trackStatus);
        return;
    }

    (void)requestTracker_.supersedePendingFor(command, generation);

    const std::optional<PlaybackRequestRecord> trackedRecord = requestTracker_.record(
        command.requestId());
    QString error;
    if (!backend_->submit(command, generation, &error)) {
        (void)requestTracker_.cancel(
            command.requestId(),
            PlaybackRequestCancellationReason::SubmissionFailed);
        if (trackedRecord.has_value()) {
            emit requestFailed(
                static_cast<quint8>(trackedRecord->type),
                error);
        }
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
