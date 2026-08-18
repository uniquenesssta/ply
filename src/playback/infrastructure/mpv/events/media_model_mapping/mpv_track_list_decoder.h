#pragma once

#include "playback/domain/models/track_descriptor.h"

#include <QList>

#include <optional>

class QString;
class QVariant;

namespace player::playback::mpv {

class MpvTrackListDecoder final
{
public:
    [[nodiscard]] static std::optional<QList<player::playback::domain::TrackDescriptor>> decode(
        const QVariant& node,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
