#include "playback/domain/state/playback_selectors.h"

namespace player::playback::domain::selectors {

bool isPlaying(const PlaybackSnapshot& snapshot) noexcept
{
    return snapshot.lifecycle() == PlaybackLifecycleState::Ready
        && snapshot.transport() == PlaybackTransportState::Playing;
}

bool canPlay(const PlaybackSnapshot& snapshot) noexcept
{
    if (snapshot.lifecycle() != PlaybackLifecycleState::Ready) {
        return false;
    }

    return snapshot.transport() == PlaybackTransportState::Idle
        || snapshot.transport() == PlaybackTransportState::Paused;
}

bool canPause(const PlaybackSnapshot& snapshot) noexcept
{
    return isPlaying(snapshot);
}

bool canStop(const PlaybackSnapshot& snapshot) noexcept
{
    if (snapshot.lifecycle() == PlaybackLifecycleState::Opening) {
        return true;
    }
    if (snapshot.lifecycle() != PlaybackLifecycleState::Ready) {
        return false;
    }

    return snapshot.transport() != PlaybackTransportState::Stopped;
}

} // namespace player::playback::domain::selectors
