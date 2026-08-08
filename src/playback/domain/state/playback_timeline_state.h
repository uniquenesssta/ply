#pragma once

#include <optional>

namespace player::playback::domain {

struct PlaybackTimelineState final
{
    std::optional<double> positionSeconds;
    std::optional<double> durationSeconds;
    std::optional<bool> seekable;
    std::optional<bool> seeking;
};

} // namespace player::playback::domain
