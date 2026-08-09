#include "playback_session.h"

#include "backend/playback_session_backend.h"
#include "playback/domain/errors/playback_failure.h"
#include "playback/domain/state/playback_invariants.h"
#include "playback/domain/state/playback_reducer.h"

#include <QThread>

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

    QString error;
    if (!backend_->submit(command, &error)) {
        commitSubmissionFailure(command, std::move(error));
    }
}

void PlaybackSession::handleBackendEvent(const PlaybackEvent& event)
{
    if (!isOnOwningThread() || !initialized_ || stopping_) {
        return;
    }
    if (!shouldApplyBackendEvent(snapshot_, event)) {
        return;
    }

    commitSnapshot(reducePlaybackSnapshot(snapshot_, event));
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

    PlaybackSnapshotState seededState = snapshot_.state();
    seededState.generation = generation;
    seededState.media.source = load->source;

    const PlaybackSnapshot opening = reducePlaybackSnapshot(
        PlaybackSnapshot{std::move(seededState)},
        PlaybackEvent{MediaLoadStartedEvent{}});
    commitSnapshot(opening);

    QString error;
    if (!backend_->submit(command, &error)) {
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
