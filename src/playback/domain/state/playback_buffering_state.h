#pragma once

#include "playback/domain/models/cache_status.h"

#include <optional>

namespace player::playback::domain {

struct PlaybackBufferingState final
{
    bool active = false;
    std::optional<double> progressPercent;
    std::optional<CacheStatus> cache;

    bool operator==(const PlaybackBufferingState&) const = default;
};

} // namespace player::playback::domain
