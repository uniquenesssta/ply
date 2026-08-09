#pragma once

#include "playback/domain/events/playback_event.h"
#include "playback/infrastructure/mpv/properties/mpv_property_change.h"

#include <optional>

class QString;

namespace player::playback::mpv {

class MpvTrackModelMapper final
{
public:
    [[nodiscard]] static std::optional<player::playback::domain::PlaybackEvent> mapTrackList(
        const MpvPropertyValue& value,
        QString* errorMessage);
    [[nodiscard]] static std::optional<player::playback::domain::PlaybackEvent> mapSelection(
        MpvPropertyId id,
        const MpvPropertyValue& value,
        QString* errorMessage);
};

} // namespace player::playback::mpv
