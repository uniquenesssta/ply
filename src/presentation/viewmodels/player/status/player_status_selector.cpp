#include "presentation/viewmodels/player/status/player_status_selector.h"

namespace player::presentation {

PlayerStatusKind selectPlayerStatus(
    const player::playback::domain::PlaybackSnapshot& snapshot) noexcept
{
    using player::playback::domain::PlaybackLifecycleState;
    using player::playback::domain::PlaybackTransportState;

    switch (snapshot.lifecycle()) {
    case PlaybackLifecycleState::Failed:
        return PlayerStatusKind::Error;
    case PlaybackLifecycleState::Opening:
        return PlayerStatusKind::Loading;
    case PlaybackLifecycleState::Ended:
        return PlayerStatusKind::Ended;
    case PlaybackLifecycleState::Ready:
        if (snapshot.buffering().active
            && snapshot.transport() != PlaybackTransportState::Paused) {
            return PlayerStatusKind::Buffering;
        }
        return PlayerStatusKind::None;
    case PlaybackLifecycleState::Empty:
        return PlayerStatusKind::Empty;
    case PlaybackLifecycleState::Closing:
        return PlayerStatusKind::None;
    }

    return PlayerStatusKind::None;
}

} // namespace player::presentation
