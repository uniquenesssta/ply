#pragma once

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

} // namespace player::playback::domain
