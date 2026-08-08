#pragma once

namespace player::playback::domain {

struct SetSpeedCommand final
{
    double rate = 1.0;
};

} // namespace player::playback::domain
