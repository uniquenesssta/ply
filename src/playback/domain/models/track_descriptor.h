#pragma once

#include <QString>
#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

enum class TrackKind : quint8
{
    Video,
    Audio,
    Subtitle,
};

struct TrackDescriptor final
{
    qint64 id = 0;
    TrackKind kind = TrackKind::Video;
    std::optional<QString> title;
    std::optional<QString> language;
    std::optional<QString> codec;
    std::optional<QString> externalFilename;
    bool selected = false;
    bool defaultTrack = false;
    bool forced = false;
    bool external = false;
    bool image = false;
    bool albumArt = false;

    bool operator==(const TrackDescriptor&) const = default;
};

} // namespace player::playback::domain
