#pragma once

#include "playback/domain/models/audio_stream_info.h"
#include "playback/domain/models/video_stream_info.h"

#include <optional>

namespace player::playback::domain {

struct PlaybackStreamState final
{
    std::optional<VideoStreamInfo> video;
    std::optional<AudioStreamInfo> audio;

    bool operator==(const PlaybackStreamState&) const = default;
};

} // namespace player::playback::domain
