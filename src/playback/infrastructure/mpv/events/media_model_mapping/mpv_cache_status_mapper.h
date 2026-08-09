#pragma once

#include "playback/domain/events/playback_event.h"
#include "playback/infrastructure/mpv/properties/mpv_property_change.h"

#include <optional>

class QString;

namespace player::playback::mpv {

class MpvCacheStatusMapper final
{
public:
    [[nodiscard]] static std::optional<player::playback::domain::PlaybackEvent> map(
        const MpvPropertyValue& value,
        QString* errorMessage);
};

} // namespace player::playback::mpv
