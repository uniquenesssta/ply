#include "playback_reducer.h"

#include <type_traits>
#include <utility>
#include <variant>

namespace player::playback::domain {
namespace {

void clearForOpening(PlaybackSnapshotState& state)
{
    const std::optional<QString> source = state.media.source;
    state.lifecycle = PlaybackLifecycleState::Opening;
    state.transport = PlaybackTransportState::Idle;
    state.media = {};
    state.media.source = source;
    state.timeline = {};
    state.buffering = {};
    state.failure.reset();
}

void clearStoppedMedia(PlaybackSnapshotState& state)
{
    state.lifecycle = PlaybackLifecycleState::Empty;
    state.transport = PlaybackTransportState::Stopped;
    state.media = {};
    state.timeline = {};
    state.buffering = {};
    state.failure.reset();
}

void markEnded(PlaybackSnapshotState& state)
{
    state.lifecycle = PlaybackLifecycleState::Ended;
    state.transport = PlaybackTransportState::Stopped;
    state.timeline.seeking = false;
    state.buffering = {};
    state.failure.reset();
}

void markMediaFailed(PlaybackSnapshotState& state, const PlaybackFailure& failure)
{
    const std::optional<QString> source = state.media.source;
    state.lifecycle = PlaybackLifecycleState::Failed;
    state.transport = PlaybackTransportState::Stopped;
    state.media = {};
    state.media.source = source;
    state.timeline = {};
    state.buffering = {};
    state.failure = failure;
}

} // namespace

PlaybackSnapshot reducePlaybackSnapshot(
    const PlaybackSnapshot& current,
    const PlaybackEvent& event)
{
    PlaybackSnapshotState state = current.state();

    std::visit(
        [&state](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;

            if constexpr (std::is_same_v<Payload, PlaybackBackendShutdownEvent>) {
                state.lifecycle = PlaybackLifecycleState::Closing;
                state.transport = PlaybackTransportState::Stopped;
                state.timeline.seeking = false;
                state.buffering = {};
            } else if constexpr (std::is_same_v<Payload, MediaLoadStartedEvent>) {
                clearForOpening(state);
            } else if constexpr (std::is_same_v<Payload, MediaLoadedEvent>) {
                state.lifecycle = PlaybackLifecycleState::Ready;
                state.failure.reset();
            } else if constexpr (std::is_same_v<Payload, MediaEndedEvent>) {
                switch (payload.reason) {
                case MediaEndReason::Stopped:
                case MediaEndReason::Shutdown:
                    clearStoppedMedia(state);
                    break;
                case MediaEndReason::Redirected:
                    clearForOpening(state);
                    break;
                case MediaEndReason::Eof:
                case MediaEndReason::Unknown:
                    markEnded(state);
                    break;
                }
            } else if constexpr (std::is_same_v<Payload, MediaFailedEvent>) {
                markMediaFailed(state, payload.failure);
            } else if constexpr (std::is_same_v<Payload, MediaTitleChangedEvent>) {
                state.media.title = payload.title;
            } else if constexpr (std::is_same_v<Payload, MediaPathChangedEvent>) {
                state.media.path = payload.path;
            } else if constexpr (std::is_same_v<Payload, PositionChangedEvent>) {
                state.timeline.positionSeconds = payload.seconds;
            } else if constexpr (std::is_same_v<Payload, DurationChangedEvent>) {
                state.timeline.durationSeconds = payload.seconds;
            } else if constexpr (std::is_same_v<Payload, SeekableChangedEvent>) {
                state.timeline.seekable = payload.seekable;
            } else if constexpr (std::is_same_v<Payload, SeekingChangedEvent>) {
                state.timeline.seeking = payload.seeking;
            } else if constexpr (std::is_same_v<Payload, BufferingChangedEvent>) {
                if (payload.buffering.has_value()) {
                    state.buffering.active = *payload.buffering;
                    if (!*payload.buffering) {
                        state.buffering.progressPercent.reset();
                    }
                }
            } else if constexpr (std::is_same_v<Payload, BufferingProgressChangedEvent>) {
                state.buffering.progressPercent = payload.percent;
            } else if constexpr (std::is_same_v<Payload, PauseChangedEvent>) {
                if (payload.paused.has_value()) {
                    state.transport = *payload.paused
                        ? PlaybackTransportState::Paused
                        : PlaybackTransportState::Playing;
                }
            } else if constexpr (std::is_same_v<Payload, VolumeChangedEvent>) {
                state.controls.volumePercent = payload.percent;
            } else if constexpr (std::is_same_v<Payload, MuteChangedEvent>) {
                state.controls.muted = payload.muted;
            } else if constexpr (std::is_same_v<Payload, SpeedChangedEvent>) {
                state.controls.speed = payload.rate;
            } else if constexpr (std::is_same_v<Payload, PlaybackFailureEvent>) {
                state.failure = payload.failure;
            } else if constexpr (
                std::is_same_v<Payload, CoreIdleChangedEvent>
                || std::is_same_v<Payload, EofReachedChangedEvent>
                || std::is_same_v<Payload, CommandReplyEvent>) {
                // These events carry backend/request observations, but they do not own
                // PlaybackSnapshot truth at R3-03. Later tasks consume them where their
                // responsibilities live (RequestTracker / Session policy).
            }
        },
        event.payload);

    return PlaybackSnapshot{std::move(state)};
}

} // namespace player::playback::domain
