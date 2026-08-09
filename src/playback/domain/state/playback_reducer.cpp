#include "playback_reducer.h"

#include <algorithm>
#include <type_traits>
#include <utility>
#include <variant>

namespace player::playback::domain {
namespace {

void clearMediaDetails(PlaybackSnapshotState& state)
{
    state.timeline = {};
    state.buffering = {};
    state.capabilities = {};
    state.streams = {};
    state.tracks = {};
    state.chapters = {};
}

void clearForOpening(PlaybackSnapshotState& state)
{
    const std::optional<QString> source = state.media.source;
    state.lifecycle = PlaybackLifecycleState::Opening;
    state.transport = PlaybackTransportState::Idle;
    state.media = {};
    state.media.source = source;
    clearMediaDetails(state);
    state.failure.reset();
}

void clearStoppedMedia(PlaybackSnapshotState& state)
{
    state.lifecycle = PlaybackLifecycleState::Empty;
    state.transport = PlaybackTransportState::Stopped;
    state.media = {};
    clearMediaDetails(state);
    state.failure.reset();
}

void markEnded(PlaybackSnapshotState& state)
{
    state.lifecycle = PlaybackLifecycleState::Ended;
    state.transport = PlaybackTransportState::Stopped;
    state.timeline.seeking = false;
    state.buffering.active = false;
    state.buffering.progressPercent.reset();
    state.buffering.cache.reset();
    state.failure.reset();
}

void markMediaFailed(PlaybackSnapshotState& state, const PlaybackFailure& failure)
{
    const std::optional<QString> source = state.media.source;
    state.lifecycle = PlaybackLifecycleState::Failed;
    state.transport = PlaybackTransportState::Stopped;
    state.media = {};
    state.media.source = source;
    clearMediaDetails(state);
    state.failure = failure;
}

bool trackExists(
    const PlaybackTrackState& state,
    qint64 id,
    TrackKind kind)
{
    return std::any_of(
        state.tracks.cbegin(),
        state.tracks.cend(),
        [id, kind](const TrackDescriptor& track) {
            return track.id == id && track.kind == kind;
        });
}

void applySelectedTrack(
    PlaybackTrackState& state,
    std::optional<qint64>& selected,
    const std::optional<qint64>& incoming,
    TrackKind kind)
{
    if (!incoming.has_value()) {
        selected.reset();
        return;
    }
    if (trackExists(state, *incoming, kind)) {
        selected = incoming;
    }
}

void rebuildTrackSelectionsAndCapabilities(PlaybackSnapshotState& state)
{
    state.tracks.selectedVideoId.reset();
    state.tracks.selectedAudioId.reset();
    state.tracks.selectedSubtitleId.reset();
    state.capabilities.hasVideoTrack = false;
    state.capabilities.hasAudioTrack = false;
    state.capabilities.hasSubtitleTrack = false;

    for (const TrackDescriptor& track : state.tracks.tracks) {
        switch (track.kind) {
        case TrackKind::Video:
            state.capabilities.hasVideoTrack = true;
            if (track.selected && !state.tracks.selectedVideoId.has_value()) {
                state.tracks.selectedVideoId = track.id;
            }
            break;
        case TrackKind::Audio:
            state.capabilities.hasAudioTrack = true;
            if (track.selected && !state.tracks.selectedAudioId.has_value()) {
                state.tracks.selectedAudioId = track.id;
            }
            break;
        case TrackKind::Subtitle:
            state.capabilities.hasSubtitleTrack = true;
            if (track.selected && !state.tracks.selectedSubtitleId.has_value()) {
                state.tracks.selectedSubtitleId = track.id;
            }
            break;
        }
    }
}

} // namespace

PlaybackSnapshot reducePlaybackSnapshot(
    const PlaybackSnapshot& current,
    const PlaybackEvent& event)
{
    PlaybackSnapshotState state = current.state();

    std::visit(
        [&state](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;

            if constexpr (std::is_same_v<Payload, PlaybackBackendShutdownEvent>) {
                state.lifecycle = PlaybackLifecycleState::Closing;
                state.transport = PlaybackTransportState::Stopped;
                state.timeline.seeking = false;
                state.buffering = {};
            } else if constexpr (std::is_same_v<Payload, MediaLoadStartedEvent>) {
                clearForOpening(state);
            } else if constexpr (std::is_same_v<Payload, MediaLoadedEvent>) {
                state.lifecycle = PlaybackLifecycleState::Ready;
                state.failure.reset();
            } else if constexpr (std::is_same_v<Payload, MediaEndedEvent>) {
                switch (payload.reason) {
                case MediaEndReason::Stopped:
                case MediaEndReason::Shutdown:
                    clearStoppedMedia(state);
                    break;
                case MediaEndReason::Redirected:
                    clearForOpening(state);
                    break;
                case MediaEndReason::Eof:
                case MediaEndReason::Unknown:
                    markEnded(state);
                    break;
                }
            } else if constexpr (std::is_same_v<Payload, MediaFailedEvent>) {
                markMediaFailed(state, payload.failure);
            } else if constexpr (std::is_same_v<Payload, MediaTitleChangedEvent>) {
                state.media.title = payload.title;
            } else if constexpr (std::is_same_v<Payload, MediaPathChangedEvent>) {
                state.media.path = payload.path;
            } else if constexpr (std::is_same_v<Payload, PositionChangedEvent>) {
                state.timeline.positionSeconds = payload.seconds;
            } else if constexpr (std::is_same_v<Payload, DurationChangedEvent>) {
                state.timeline.durationSeconds = payload.seconds;
            } else if constexpr (std::is_same_v<Payload, SeekableChangedEvent>) {
                state.timeline.seekable = payload.seekable;
            } else if constexpr (std::is_same_v<Payload, SeekingChangedEvent>) {
                state.timeline.seeking = payload.seeking;
            } else if constexpr (std::is_same_v<Payload, BufferingChangedEvent>) {
                if (payload.buffering.has_value()) {
                    state.buffering.active = *payload.buffering;
                    if (!*payload.buffering) {
                        state.buffering.progressPercent.reset();
                    }
                }
            } else if constexpr (std::is_same_v<Payload, BufferingProgressChangedEvent>) {
                state.buffering.progressPercent = payload.percent;
            } else if constexpr (std::is_same_v<Payload, CacheStatusChangedEvent>) {
                state.buffering.cache = payload.status;
            } else if constexpr (std::is_same_v<Payload, PauseChangedEvent>) {
                if (payload.paused.has_value()) {
                    state.transport = *payload.paused
                        ? PlaybackTransportState::Paused
                        : PlaybackTransportState::Playing;
                }
            } else if constexpr (std::is_same_v<Payload, VolumeChangedEvent>) {
                state.controls.volumePercent = payload.percent;
            } else if constexpr (std::is_same_v<Payload, MuteChangedEvent>) {
                state.controls.muted = payload.muted;
            } else if constexpr (std::is_same_v<Payload, SpeedChangedEvent>) {
                state.controls.speed = payload.rate;
            } else if constexpr (std::is_same_v<Payload, TrackListChangedEvent>) {
                state.tracks.tracks = payload.tracks;
                rebuildTrackSelectionsAndCapabilities(state);
            } else if constexpr (std::is_same_v<Payload, SelectedVideoTrackChangedEvent>) {
                applySelectedTrack(
                    state.tracks,
                    state.tracks.selectedVideoId,
                    payload.trackId,
                    TrackKind::Video);
            } else if constexpr (std::is_same_v<Payload, SelectedAudioTrackChangedEvent>) {
                applySelectedTrack(
                    state.tracks,
                    state.tracks.selectedAudioId,
                    payload.trackId,
                    TrackKind::Audio);
            } else if constexpr (std::is_same_v<Payload, SelectedSubtitleTrackChangedEvent>) {
                applySelectedTrack(
                    state.tracks,
                    state.tracks.selectedSubtitleId,
                    payload.trackId,
                    TrackKind::Subtitle);
            } else if constexpr (std::is_same_v<Payload, ChapterListChangedEvent>) {
                state.chapters.chapters = payload.chapters;
                state.capabilities.hasChapters = !payload.chapters.isEmpty();
            } else if constexpr (std::is_same_v<Payload, VideoStreamInfoChangedEvent>) {
                state.streams.video = payload.info;
            } else if constexpr (std::is_same_v<Payload, AudioStreamInfoChangedEvent>) {
                state.streams.audio = payload.info;
            } else if constexpr (std::is_same_v<Payload, PlaybackFailureEvent>) {
                state.failure = payload.failure;
            } else if constexpr (
                std::is_same_v<Payload, CoreIdleChangedEvent>
                || std::is_same_v<Payload, EofReachedChangedEvent>
                || std::is_same_v<Payload, CommandReplyEvent>) {
                // These observations are consumed by Session/request policy rather than
                // owning PlaybackSnapshot state directly.
            }
        },
        event.payload);

    return PlaybackSnapshot{std::move(state)};
}

} // namespace player::playback::domain
