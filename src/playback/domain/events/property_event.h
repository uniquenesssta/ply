#pragma once

#include <optional>

namespace player::playback::domain {

struct PauseChangedEvent final
{
    std::optional<bool> paused;
};

struct VolumeChangedEvent final
{
    std::optional<double> percent;
};

struct MuteChangedEvent final
{
    std::optional<bool> muted;
};

struct SpeedChangedEvent final
{
    std::optional<double> rate;
};

struct CoreIdleChangedEvent final
{
    std::optional<bool> idle;
};

struct EofReachedChangedEvent final
{
    std::optional<bool> reached;
};

struct SubtitleDelayChangedEvent final
{
    std::optional<double> seconds;
};

} // namespace player::playback::domain
