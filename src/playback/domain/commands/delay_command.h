#pragma once

namespace player::playback::domain {

// Subtitle delay in seconds (positive = subtitle shown later, negative =
// subtitle shown earlier). Kept as an independent state axis from audio delay.
struct SetSubtitleDelayCommand final
{
    double seconds = 0.0;
};

// Audio delay in seconds (positive = audio shifted later). Independent from
// subtitle delay; the two never share state or a single command.
struct SetAudioDelayCommand final
{
    double seconds = 0.0;
};

} // namespace player::playback::domain
