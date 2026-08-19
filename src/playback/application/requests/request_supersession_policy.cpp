#include "request_supersession_policy.h"

namespace player::playback::application {

std::optional<PlaybackRequestSupersessionGroup> requestSupersessionGroupFor(
    PlaybackRequestType type) noexcept
{
    switch (type) {
    case PlaybackRequestType::LoadMedia:
        return PlaybackRequestSupersessionGroup::LoadMedia;
    case PlaybackRequestType::SeekAbsolute:
    case PlaybackRequestType::SeekRelative:
        return PlaybackRequestSupersessionGroup::Seek;
    case PlaybackRequestType::SelectAudioTrack:
        return PlaybackRequestSupersessionGroup::AudioTrackSelection;
    case PlaybackRequestType::SelectSubtitleTrack:
        return PlaybackRequestSupersessionGroup::SubtitleTrackSelection;
    case PlaybackRequestType::SelectVideoTrack:
        return PlaybackRequestSupersessionGroup::VideoTrackSelection;
    case PlaybackRequestType::Play:
    case PlaybackRequestType::Pause:
    case PlaybackRequestType::Stop:
    case PlaybackRequestType::SetVolume:
    case PlaybackRequestType::SetMuted:
    case PlaybackRequestType::SetSpeed:
    case PlaybackRequestType::AddExternalSubtitle:
        return std::nullopt;
    }
    return std::nullopt;
}

bool shouldSupersedeRequest(
    const PlaybackRequestRecord& pending,
    PlaybackRequestType replacementType,
    player::playback::domain::MediaGeneration replacementGeneration) noexcept
{
    if (pending.state != PlaybackRequestState::Pending) {
        return false;
    }

    const auto pendingGroup = requestSupersessionGroupFor(pending.type);
    const auto replacementGroup = requestSupersessionGroupFor(replacementType);
    if (!pendingGroup.has_value()
        || !replacementGroup.has_value()
        || *pendingGroup != *replacementGroup) {
        return false;
    }

    if (*replacementGroup == PlaybackRequestSupersessionGroup::LoadMedia) {
        return true;
    }

    return pending.generation.has_value()
        && *pending.generation == replacementGeneration;
}

} // namespace player::playback::application
