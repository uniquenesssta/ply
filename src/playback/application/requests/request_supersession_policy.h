#pragma once

#include "playback/application/requests/playback_request.h"

#include <optional>

namespace player::playback::application {

enum class PlaybackRequestSupersessionGroup : quint8
{
    LoadMedia,
    Seek,
    AudioTrackSelection,
    SubtitleTrackSelection,
    VideoTrackSelection,
};

[[nodiscard]] std::optional<PlaybackRequestSupersessionGroup> requestSupersessionGroupFor(
    PlaybackRequestType type) noexcept;

[[nodiscard]] bool shouldSupersedeRequest(
    const PlaybackRequestRecord& pending,
    PlaybackRequestType replacementType,
    player::playback::domain::MediaGeneration replacementGeneration) noexcept;

} // namespace player::playback::application
