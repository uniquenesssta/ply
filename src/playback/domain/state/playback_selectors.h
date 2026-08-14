#pragma once

#include "playback/domain/state/playback_snapshot.h"

namespace player::playback::domain::selectors {

[[nodiscard]] bool isPlaying(const PlaybackSnapshot& snapshot) noexcept;
[[nodiscard]] bool canPlay(const PlaybackSnapshot& snapshot) noexcept;
[[nodiscard]] bool canPause(const PlaybackSnapshot& snapshot) noexcept;
[[nodiscard]] bool canStop(const PlaybackSnapshot& snapshot) noexcept;

} // namespace player::playback::domain::selectors
