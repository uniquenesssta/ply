#pragma once

#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

enum class TrackSelectionKind : quint8
{
    Audio,
    Subtitle,
};

struct TrackSelectionCommand final
{
    TrackSelectionKind kind = TrackSelectionKind::Audio;
    std::optional<qint64> trackId;
};

} // namespace player::playback::domain
