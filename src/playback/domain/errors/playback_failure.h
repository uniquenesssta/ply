#pragma once

#include <QString>
#include <QtGlobal>

namespace player::playback::domain {

enum class PlaybackFailureCategory : quint8
{
    Command,
    Media,
    Backend,
    Protocol,
};

struct PlaybackFailure final
{
    PlaybackFailureCategory category = PlaybackFailureCategory::Backend;
    int backendCode = 0;
    QString diagnostic;
};

} // namespace player::playback::domain
