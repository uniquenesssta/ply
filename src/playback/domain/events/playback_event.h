#pragma once

#include "playback/domain/events/buffering_event.h"
#include "playback/domain/events/chapter_event.h"
#include "playback/domain/events/command_event.h"
#include "playback/domain/events/failure_event.h"
#include "playback/domain/events/lifecycle_event.h"
#include "playback/domain/events/media_event.h"
#include "playback/domain/events/position_event.h"
#include "playback/domain/events/property_event.h"
#include "playback/domain/events/stream_event.h"
#include "playback/domain/events/track_event.h"
#include "playback/domain/state/media_generation.h"

#include <variant>

namespace player::playback::domain {

using PlaybackEventPayload = std::variant<
    PlaybackBackendShutdownEvent,
    MediaLoadStartedEvent,
    MediaLoadedEvent,
    MediaEndedEvent,
    MediaFailedEvent,
    MediaTitleChangedEvent,
    MediaPathChangedEvent,
    PositionChangedEvent,
    DurationChangedEvent,
    SeekableChangedEvent,
    SeekingChangedEvent,
    BufferingChangedEvent,
    BufferingProgressChangedEvent,
    CacheStatusChangedEvent,
    PauseChangedEvent,
    VolumeChangedEvent,
    MuteChangedEvent,
    SpeedChangedEvent,
    CoreIdleChangedEvent,
    EofReachedChangedEvent,
    TrackListChangedEvent,
    SelectedVideoTrackChangedEvent,
    SelectedAudioTrackChangedEvent,
    SelectedSubtitleTrackChangedEvent,
    ChapterListChangedEvent,
    VideoStreamInfoChangedEvent,
    AudioStreamInfoChangedEvent,
    CommandReplyEvent,
    PlaybackFailureEvent,
    SubtitleDelayChangedEvent,
    AudioDelayChangedEvent>;

struct PlaybackEvent final
{
    PlaybackEventPayload payload;
    MediaGeneration generation{};
};

} // namespace player::playback::domain
