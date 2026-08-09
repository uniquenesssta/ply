#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QtGlobal>

#include <vector>

namespace player::playback::domain {

enum class PlaybackInvariantViolation : quint8
{
    EmptyHasMediaScopedState,
    ActiveMediaMissingGeneration,
    ActiveMediaMissingSource,
    LifecycleTransportMismatch,
    FailedWithoutFailure,
    BufferingOutsideActiveMedia,
    SeekingOutsideReadyMedia,
    SeekingWhenNotSeekable,
    GenerationRegressed,
};

using PlaybackInvariantViolations = std::vector<PlaybackInvariantViolation>;

[[nodiscard]] PlaybackInvariantViolations checkPlaybackSnapshotInvariants(
    const PlaybackSnapshot& snapshot);

[[nodiscard]] PlaybackInvariantViolations checkPlaybackTransitionInvariants(
    const PlaybackSnapshot& previous,
    const PlaybackSnapshot& next);

} // namespace player::playback::domain
