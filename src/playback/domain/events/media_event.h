#pragma once

#include <QString>
#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

enum class MediaEndReason : quint8
{
    Eof,
    Stopped,
    Shutdown,
    Redirected,
    Unknown,
};

struct MediaLoadStartedEvent final
{
};

struct MediaLoadedEvent final
{
};

struct MediaEndedEvent final
{
    MediaEndReason reason = MediaEndReason::Unknown;
};

struct MediaTitleChangedEvent final
{
    std::optional<QString> title;
};

struct MediaPathChangedEvent final
{
    std::optional<QString> path;
};

} // namespace player::playback::domain
