#pragma once

#include "playback/domain/events/playback_event.h"
#include "playback/domain/state/playback_snapshot.h"

namespace player::playback::domain {

[[nodiscard]] PlaybackSnapshot reducePlaybackSnapshot(
    const PlaybackSnapshot& current,
    const PlaybackEvent& event);

} // namespace player::playback::domain
