#pragma once

#include "playback/domain/models/track_descriptor.h"

#include <optional>

namespace player::playback::domain {

// Selects one backend track by its stable backend id, or disables the track
// family when trackId is empty. Disabling is a first-class state (e.g. turning
// subtitles off); it is never represented by a non-existent index.
struct SelectTrackCommand final
{
    TrackKind kind = TrackKind::Video;
    std::optional<qint64> trackId;
};

} // namespace player::playback::domain
