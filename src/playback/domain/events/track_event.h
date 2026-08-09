#pragma once

#include "playback/domain/models/track_descriptor.h"

#include <QList>
#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

struct TrackListChangedEvent final
{
    QList<TrackDescriptor> tracks;
};

struct SelectedVideoTrackChangedEvent final
{
    std::optional<qint64> trackId;
};

struct SelectedAudioTrackChangedEvent final
{
    std::optional<qint64> trackId;
};

struct SelectedSubtitleTrackChangedEvent final
{
    std::optional<qint64> trackId;
};

} // namespace player::playback::domain
