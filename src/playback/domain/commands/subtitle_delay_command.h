#pragma once

namespace player::playback::domain {

inline constexpr double kSubtitleDelayMinimumSeconds = -2.0;
inline constexpr double kSubtitleDelayMaximumSeconds = 2.0;
inline constexpr double kSubtitleDelayStepSeconds = 0.05;

struct SetSubtitleDelayCommand final
{
    double seconds = 0.0;
};

} // namespace player::playback::domain
