#pragma once

#include "playback/domain/errors/playback_failure.h"
#include "playback/domain/state/playback_snapshot.h"

namespace player::playback::domain::reducer_detail {

void prepareForOpening(PlaybackSnapshotState& state);
void clearForStop(PlaybackSnapshotState& state);
void markEnded(PlaybackSnapshotState& state);
void markFailed(PlaybackSnapshotState& state, const PlaybackFailure& failure);

} // namespace player::playback::domain::reducer_detail
