#pragma once

#include "playback/domain/models/audio_stream_info.h"
#include "playback/domain/models/video_stream_info.h"

#include <optional>

namespace player::playback::domain {

struct VideoStreamInfoChangedEvent final
{
    std::optional<VideoStreamInfo> info;
};

struct AudioStreamInfoChangedEvent final
{
    std::optional<AudioStreamInfo> info;
};

} // namespace player::playback::domain
