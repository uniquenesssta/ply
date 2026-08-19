#pragma once

namespace player::playback::domain {

inline constexpr double kAudioDelayMinimumSeconds = -2.0;
inline constexpr double kAudioDelayMaximumSeconds = 2.0;
inline constexpr double kAudioDelayStepSeconds = 0.05;

struct SetAudioDelayCommand final
{
    double seconds = 0.0;
};

} // namespace player::playback::domain
