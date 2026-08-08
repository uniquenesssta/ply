#pragma once

#include <QtGlobal>

namespace player::playback::domain {

enum class SeekMode : quint8
{
    Absolute,
    Relative,
};

struct SeekCommand final
{
    double seconds = 0.0;
    SeekMode mode = SeekMode::Absolute;
};

} // namespace player::playback::domain
