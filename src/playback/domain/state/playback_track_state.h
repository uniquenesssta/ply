#pragma once

#include "playback/domain/models/track_descriptor.h"

#include <QList>
#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

struct PlaybackTrackState final
{
    QList<TrackDescriptor> tracks;
    std::optional<qint64> selectedVideoId;
    std::optional<qint64> selectedAudioId;
    std::optional<qint64> selectedSubtitleId;

    bool operator==(const PlaybackTrackState&) const = default;
};

} // namespace player::playback::domain
