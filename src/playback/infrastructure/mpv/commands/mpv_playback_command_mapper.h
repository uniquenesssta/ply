#pragma once

#include "playback/domain/commands/playback_command.h"
#include "playback/infrastructure/mpv/commands/mpv_command_request.h"

#include <optional>

namespace player::playback::mpv {

class MpvPlaybackCommandMapper final
{
public:
    [[nodiscard]] static std::optional<MpvCommandRequest> map(
        const player::playback::domain::PlaybackCommandPayload& payload);
};

} // namespace player::playback::mpv
