#pragma once

#include <QString>

#include <optional>

namespace player::playback::domain {

struct PlaybackMediaState final
{
    std::optional<QString> source;
    std::optional<QString> title;
    std::optional<QString> path;
};

} // namespace player::playback::domain
