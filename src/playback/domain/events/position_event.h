#pragma once

#include <optional>

namespace player::playback::domain {

struct PositionChangedEvent final
{
    std::optional<double> seconds;
};

struct DurationChangedEvent final
{
    std::optional<double> seconds;
};

struct SeekableChangedEvent final
{
    std::optional<bool> seekable;
};

struct SeekingChangedEvent final
{
    std::optional<bool> seeking;
};

} // namespace player::playback::domain
