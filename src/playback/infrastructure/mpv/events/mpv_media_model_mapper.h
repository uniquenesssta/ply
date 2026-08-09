#pragma once

#include "playback/domain/events/playback_event.h"
#include "playback/infrastructure/mpv/properties/mpv_property_change.h"

#include <optional>

class QString;

namespace player::playback::mpv {

class MpvMediaModelMapper final
{
public:
    [[nodiscard]] static std::optional<player::playback::domain::PlaybackEvent> map(
        const MpvPropertyChange& change,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
