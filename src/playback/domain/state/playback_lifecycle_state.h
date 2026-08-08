#pragma once

#include <QtGlobal>

namespace player::playback::domain {

enum class PlaybackLifecycleState : quint8
{
    Empty,
    Opening,
    Ready,
    Ended,
    Failed,
    Closing,
};

} // namespace player::playback::domain
