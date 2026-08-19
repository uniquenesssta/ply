#pragma once

#include <optional>

namespace player::playback::domain {

struct PlaybackControlsState final
{
    std::optional<double> volumePercent;
    std::optional<bool> muted;
    std::optional<double> speed;
    std::optional<double> subtitleDelaySeconds;
    std::optional<double> audioDelaySeconds;
};

} // namespace player::playback::domain
