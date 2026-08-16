#pragma once

#include <QtGlobal>

namespace player::playlist::domain {

enum class PlaylistRepeatMode : quint8
{
    Off = 0,
    One,
    All,
};

} // namespace player::playlist::domain
