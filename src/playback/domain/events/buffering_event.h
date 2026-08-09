#pragma once

#include "playback/domain/models/cache_status.h"

#include <optional>

namespace player::playback::domain {

struct BufferingChangedEvent final
{
    std::optional<bool> buffering;
};

struct BufferingProgressChangedEvent final
{
    std::optional<double> percent;
};

struct CacheStatusChangedEvent final
{
    std::optional<CacheStatus> status;
};

} // namespace player::playback::domain
