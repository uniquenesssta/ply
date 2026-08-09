#pragma once

namespace player::playback::domain {

struct PlaybackCapabilitiesState final
{
    bool hasVideoTrack = false;
    bool hasAudioTrack = false;
    bool hasSubtitleTrack = false;
    bool hasChapters = false;

    bool operator==(const PlaybackCapabilitiesState&) const = default;
};

} // namespace player::playback::domain
