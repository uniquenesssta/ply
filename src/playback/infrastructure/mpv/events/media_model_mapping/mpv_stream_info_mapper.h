#pragma once

#include "playback/domain/events/playback_event.h"
#include "playback/infrastructure/mpv/properties/mpv_property_change.h"

#include <optional>

class QString;

namespace player::playback::mpv {

class MpvStreamInfoMapper final
{
public:
    [[nodiscard]] static std::optional<player::playback::domain::PlaybackEvent> mapVideo(
        const MpvPropertyValue& value,
        QString* errorMessage);
    [[nodiscard]] static std::optional<player::playback::domain::PlaybackEvent> mapAudio(
        const MpvPropertyValue& value,
        QString* errorMessage);
};

} // namespace player::playback::mpv
