#pragma once

#include <QtGlobal>

namespace player::playback::domain {

enum class TransportAction : quint8
{
    Play,
    Pause,
    Stop,
};

struct TransportCommand final
{
    TransportAction action = TransportAction::Play;
};

} // namespace player::playback::domain
