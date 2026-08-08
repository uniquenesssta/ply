#include "playback_snapshot.h"

#include <utility>

namespace player::playback::domain {

PlaybackSnapshot::PlaybackSnapshot(PlaybackSnapshotState state)
    : state_(std::move(state))
{
}

PlaybackSnapshot PlaybackSnapshot::opening(MediaGeneration generation, QString source)
{
    PlaybackSnapshotState state;
    state.generation = generation;
    state.lifecycle = PlaybackLifecycleState::Opening;
    state.transport = PlaybackTransportState::Idle;
    state.media.source = std::move(source);
    return PlaybackSnapshot{std::move(state)};
}

PlaybackSnapshot PlaybackSnapshot::stopped(MediaGeneration generation)
{
    PlaybackSnapshotState state;
    state.generation = generation;
    state.lifecycle = PlaybackLifecycleState::Empty;
    state.transport = PlaybackTransportState::Stopped;
    return PlaybackSnapshot{std::move(state)};
}

const PlaybackSnapshotState& PlaybackSnapshot::state() const noexcept
{
    return state_;
}

MediaGeneration PlaybackSnapshot::generation() const noexcept
{
    return state_.generation;
}

PlaybackLifecycleState PlaybackSnapshot::lifecycle() const noexcept
{
    return state_.lifecycle;
}

PlaybackTransportState PlaybackSnapshot::transport() const noexcept
{
    return state_.transport;
}

const PlaybackMediaState& PlaybackSnapshot::media() const noexcept
{
    return state_.media;
}

const PlaybackTimelineState& PlaybackSnapshot::timeline() const noexcept
{
    return state_.timeline;
}

const PlaybackBufferingState& PlaybackSnapshot::buffering() const noexcept
{
    return state_.buffering;
}

const PlaybackControlsState& PlaybackSnapshot::controls() const noexcept
{
    return state_.controls;
}

const std::optional<PlaybackFailure>& PlaybackSnapshot::failure() const noexcept
{
    return state_.failure;
}

} // namespace player::playback::domain
