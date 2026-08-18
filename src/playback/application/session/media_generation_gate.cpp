#include "media_generation_gate.h"

#include "playback/domain/events/buffering_event.h"
#include "playback/domain/events/chapter_event.h"
#include "playback/domain/events/failure_event.h"
#include "playback/domain/events/lifecycle_event.h"
#include "playback/domain/events/media_event.h"
#include "playback/domain/events/position_event.h"
#include "playback/domain/events/property_event.h"
#include "playback/domain/events/stream_event.h"
#include "playback/domain/events/track_event.h"

#include <type_traits>
#include <variant>

namespace player::playback::application {

using namespace player::playback::domain;

void MediaGenerationGate::activate(MediaGeneration generation) noexcept
{
    currentGeneration_ = generation;
}

void MediaGenerationGate::reset() noexcept
{
    currentGeneration_ = {};
}

bool MediaGenerationGate::accepts(const PlaybackEvent& event) noexcept
{
    if (!isMediaScoped(event)) {
        return true;
    }

    if (!event.generation.isValid()) {
        ++diagnostics_.missingGenerationEventCount;
        return false;
    }

    if (!currentGeneration_.isValid() || event.generation != currentGeneration_) {
        ++diagnostics_.staleGenerationEventCount;
        return false;
    }

    return true;
}

MediaGeneration MediaGenerationGate::currentGeneration() const noexcept
{
    return currentGeneration_;
}

const MediaGenerationGateDiagnostics& MediaGenerationGate::diagnostics() const noexcept
{
    return diagnostics_;
}

bool MediaGenerationGate::isMediaScoped(const PlaybackEvent& event) noexcept
{
    if (std::holds_alternative<PlaybackFailureEvent>(event.payload)) {
        return event.generation.isValid();
    }

    return std::visit(
        [](const auto& payload) noexcept {
            using Payload = std::decay_t<decltype(payload)>;
            return std::is_same_v<Payload, MediaLoadStartedEvent>
                || std::is_same_v<Payload, MediaLoadedEvent>
                || std::is_same_v<Payload, MediaEndedEvent>
                || std::is_same_v<Payload, MediaFailedEvent>
                || std::is_same_v<Payload, MediaTitleChangedEvent>
                || std::is_same_v<Payload, MediaPathChangedEvent>
                || std::is_same_v<Payload, PositionChangedEvent>
                || std::is_same_v<Payload, DurationChangedEvent>
                || std::is_same_v<Payload, SeekableChangedEvent>
                || std::is_same_v<Payload, SeekingChangedEvent>
                || std::is_same_v<Payload, BufferingChangedEvent>
                || std::is_same_v<Payload, BufferingProgressChangedEvent>
                || std::is_same_v<Payload, CacheStatusChangedEvent>
                || std::is_same_v<Payload, PauseChangedEvent>
                || std::is_same_v<Payload, CoreIdleChangedEvent>
                || std::is_same_v<Payload, EofReachedChangedEvent>
                || std::is_same_v<Payload, TrackListChangedEvent>
                || std::is_same_v<Payload, SelectedVideoTrackChangedEvent>
                || std::is_same_v<Payload, SelectedAudioTrackChangedEvent>
                || std::is_same_v<Payload, SelectedSubtitleTrackChangedEvent>
                || std::is_same_v<Payload, ChapterListChangedEvent>
                || std::is_same_v<Payload, VideoStreamInfoChangedEvent>
                || std::is_same_v<Payload, AudioStreamInfoChangedEvent>
                || std::is_same_v<Payload, SubtitleDelayChangedEvent>
                || std::is_same_v<Payload, AudioDelayChangedEvent>;
        },
        event.payload);
}

} // namespace player::playback::application
