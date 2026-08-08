#pragma once

#include "playback/domain/events/playback_event.h"
#include "playback/infrastructure/mpv/events/mpv_event.h"

#include <optional>

namespace player::playback::mpv {

class MpvPlaybackEventMapper final
{
public:
    [[nodiscard]] static std::optional<player::playback::domain::PlaybackEvent> map(
        const MpvEvent& event);
};

} // namespace player::playback::mpv
