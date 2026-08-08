#pragma once

namespace player::playback::domain {

struct SetVolumeCommand final
{
    double percent = 100.0;
};

struct SetMutedCommand final
{
    bool muted = false;
};

} // namespace player::playback::domain
