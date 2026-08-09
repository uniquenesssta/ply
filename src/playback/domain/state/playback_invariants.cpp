#include "playback_invariants.h"

#include <algorithm>

namespace player::playback::domain {
namespace {

bool hasMediaScopedState(const PlaybackSnapshotState& state)
{
    return state.media.source.has_value()
        || state.media.title.has_value()
        || state.media.path.has_value()
        || state.timeline.positionSeconds.has_value()
        || state.timeline.durationSeconds.has_value()
        || state.timeline.seekable.has_value()
        || state.timeline.seeking.has_value()
        || state.buffering.active
        || state.buffering.progressPercent.has_value()
        || state.buffering.cache.has_value()
        || state.capabilities.hasVideoTrack
        || state.capabilities.hasAudioTrack
        || state.capabilities.hasSubtitleTrack
        || state.capabilities.hasChapters
        || state.streams.video.has_value()
        || state.streams.audio.has_value()
        || !state.tracks.tracks.isEmpty()
        || state.tracks.selectedVideoId.has_value()
        || state.tracks.selectedAudioId.has_value()
        || state.tracks.selectedSubtitleId.has_value()
        || !state.chapters.chapters.isEmpty();
}

bool lifecycleRequiresMediaIdentity(PlaybackLifecycleState lifecycle)
{
    switch (lifecycle) {
    case PlaybackLifecycleState::Opening:
    case PlaybackLifecycleState::Ready:
    case PlaybackLifecycleState::Ended:
    case PlaybackLifecycleState::Failed:
        return true;
    case PlaybackLifecycleState::Empty:
    case PlaybackLifecycleState::Closing:
        return false;
    }

    return false;
}

bool transportMatchesLifecycle(
    PlaybackLifecycleState lifecycle,
    PlaybackTransportState transport)
{
    switch (lifecycle) {
    case PlaybackLifecycleState::Empty:
        return transport == PlaybackTransportState::Idle
            || transport == PlaybackTransportState::Stopped;
    case PlaybackLifecycleState::Opening:
        return transport == PlaybackTransportState::Idle;
    case PlaybackLifecycleState::Ready:
        return transport == PlaybackTransportState::Idle
            || transport == PlaybackTransportState::Playing
            || transport == PlaybackTransportState::Paused;
    case PlaybackLifecycleState::Ended:
    case PlaybackLifecycleState::Failed:
    case PlaybackLifecycleState::Closing:
        return transport == PlaybackTransportState::Stopped;
    }

    return false;
}

bool selectionExists(
    const PlaybackTrackState& state,
    const std::optional<qint64>& selectedId,
    TrackKind expectedKind)
{
    if (!selectedId.has_value()) {
        return true;
    }

    return std::any_of(
        state.tracks.cbegin(),
        state.tracks.cend(),
        [selectedId, expectedKind](const TrackDescriptor& track) {
            return track.id == *selectedId && track.kind == expectedKind;
        });
}

void addViolation(
    PlaybackInvariantViolations& violations,
    PlaybackInvariantViolation violation)
{
    if (std::find(violations.cbegin(), violations.cend(), violation) == violations.cend()) {
        violations.push_back(violation);
    }
}

} // namespace

PlaybackInvariantViolations checkPlaybackSnapshotInvariants(
    const PlaybackSnapshot& snapshot)
{
    const PlaybackSnapshotState& state = snapshot.state();
    PlaybackInvariantViolations violations;

    if (state.lifecycle == PlaybackLifecycleState::Empty && hasMediaScopedState(state)) {
        addViolation(violations, PlaybackInvariantViolation::EmptyHasMediaScopedState);
    }

    if (lifecycleRequiresMediaIdentity(state.lifecycle)) {
        if (!state.generation.isValid()) {
            addViolation(violations, PlaybackInvariantViolation::ActiveMediaMissingGeneration);
        }
        if (!state.media.source.has_value() || state.media.source->isEmpty()) {
            addViolation(violations, PlaybackInvariantViolation::ActiveMediaMissingSource);
        }
    }

    if (!transportMatchesLifecycle(state.lifecycle, state.transport)) {
        addViolation(violations, PlaybackInvariantViolation::LifecycleTransportMismatch);
    }

    if (state.lifecycle == PlaybackLifecycleState::Failed && !state.failure.has_value()) {
        addViolation(violations, PlaybackInvariantViolation::FailedWithoutFailure);
    }

    if (state.buffering.active
        && state.lifecycle != PlaybackLifecycleState::Opening
        && state.lifecycle != PlaybackLifecycleState::Ready) {
        addViolation(violations, PlaybackInvariantViolation::BufferingOutsideActiveMedia);
    }

    if (state.timeline.seeking.value_or(false)) {
        if (state.lifecycle != PlaybackLifecycleState::Ready) {
            addViolation(violations, PlaybackInvariantViolation::SeekingOutsideReadyMedia);
        }
        if (state.timeline.seekable.has_value() && !*state.timeline.seekable) {
            addViolation(violations, PlaybackInvariantViolation::SeekingWhenNotSeekable);
        }
    }

    if (!selectionExists(state.tracks, state.tracks.selectedVideoId, TrackKind::Video)
        || !selectionExists(state.tracks, state.tracks.selectedAudioId, TrackKind::Audio)
        || !selectionExists(state.tracks, state.tracks.selectedSubtitleId, TrackKind::Subtitle)) {
        addViolation(violations, PlaybackInvariantViolation::SelectedTrackMissingFromCurrentMedia);
    }

    return violations;
}

PlaybackInvariantViolations checkPlaybackTransitionInvariants(
    const PlaybackSnapshot& previous,
    const PlaybackSnapshot& next)
{
    PlaybackInvariantViolations violations;
    const MediaGeneration previousGeneration = previous.generation();
    const MediaGeneration nextGeneration = next.generation();

    if (previousGeneration.isValid()
        && (!nextGeneration.isValid() || nextGeneration < previousGeneration)) {
        addViolation(violations, PlaybackInvariantViolation::GenerationRegressed);
    }

    return violations;
}

} // namespace player::playback::domain
