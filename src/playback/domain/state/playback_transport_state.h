#pragma once

#include <QtGlobal>

namespace player::playback::domain {

enum class PlaybackTransportState : quint8
{
    Idle,
    Playing,
    Paused,
    Stopped,
};

} // namespace player::playback::domain
