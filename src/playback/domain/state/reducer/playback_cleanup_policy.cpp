#include "playback_cleanup_policy.h"

#include <optional>

namespace player::playback::domain::reducer_detail {
namespace {

void clearMediaScopedDetails(PlaybackSnapshotState& state)
{
    state.timeline = {};
    state.buffering = {};
    state.capabilities = {};
    state.streams = {};
    state.tracks = {};
    state.chapters = {};
}

} // namespace

void prepareForOpening(PlaybackSnapshotState& state)
{
    const std::optional<QString> source = state.media.source;

    state.lifecycle = PlaybackLifecycleState::Opening;
    state.transport = PlaybackTransportState::Idle;
    state.media = {};
    state.media.source = source;
    clearMediaScopedDetails(state);
    state.failure.reset();
}

void clearForStop(PlaybackSnapshotState& state)
{
    state.lifecycle = PlaybackLifecycleState::Empty;
    state.transport = PlaybackTransportState::Stopped;
    state.media = {};
    clearMediaScopedDetails(state);
    state.failure.reset();
}

void markEnded(PlaybackSnapshotState& state)
{
    state.lifecycle = PlaybackLifecycleState::Ended;
    state.transport = PlaybackTransportState::Stopped;

    if (state.timeline.durationSeconds.has_value()) {
        state.timeline.positionSeconds = state.timeline.durationSeconds;
    }
    state.timeline.seeking = false;

    state.buffering.active = false;
    state.buffering.progressPercent.reset();
    state.buffering.cache.reset();
    state.failure.reset();
}

void markFailed(PlaybackSnapshotState& state, const PlaybackFailure& failure)
{
    const std::optional<QString> source = state.media.source;

    state.lifecycle = PlaybackLifecycleState::Failed;
    state.transport = PlaybackTransportState::Stopped;
    state.media = {};
    state.media.source = source;
    clearMediaScopedDetails(state);
    state.failure = failure;
}

} // namespace player::playback::domain::reducer_detail
