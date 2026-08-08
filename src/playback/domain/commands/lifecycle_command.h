#pragma once

#include <QtGlobal>

namespace player::playback::domain {

enum class PlaybackLifecycleAction : quint8
{
    Initialize,
    Shutdown,
};

struct LifecycleCommand final
{
    PlaybackLifecycleAction action = PlaybackLifecycleAction::Initialize;
};

} // namespace player::playback::domain
