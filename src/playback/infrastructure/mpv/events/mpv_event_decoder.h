#pragma once

#include "playback/infrastructure/mpv/events/mpv_event.h"

struct mpv_event;

namespace player::playback::mpv {

class MpvEventDecoder final
{
public:
    [[nodiscard]] static MpvEvent decode(const mpv_event& event);
};

} // namespace player::playback::mpv
