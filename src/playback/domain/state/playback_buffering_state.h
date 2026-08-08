#pragma once

#include <optional>

namespace player::playback::domain {

struct PlaybackBufferingState final
{
    bool active = false;
    std::optional<double> progressPercent;
};

} // namespace player::playback::domain
