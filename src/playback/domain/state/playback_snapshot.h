#pragma once

#include "playback/domain/errors/playback_failure.h"
#include "playback/domain/state/media_generation.h"
#include "playback/domain/state/playback_buffering_state.h"
#include "playback/domain/state/playback_capabilities_state.h"
#include "playback/domain/state/playback_chapter_state.h"
#include "playback/domain/state/playback_controls_state.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_media_state.h"
#include "playback/domain/state/playback_stream_state.h"
#include "playback/domain/state/playback_timeline_state.h"
#include "playback/domain/state/playback_track_state.h"
#include "playback/domain/state/playback_transport_state.h"

#include <QMetaType>
#include <QString>

#include <optional>

namespace player::playback::domain {

struct PlaybackSnapshotState final
{
    MediaGeneration generation;
    PlaybackLifecycleState lifecycle = PlaybackLifecycleState::Empty;
    PlaybackTransportState transport = PlaybackTransportState::Idle;
    PlaybackMediaState media;
    PlaybackTimelineState timeline;
    PlaybackBufferingState buffering;
    PlaybackControlsState controls;
    PlaybackCapabilitiesState capabilities;
    PlaybackStreamState streams;
    PlaybackTrackState tracks;
    PlaybackChapterState chapters;
    std::optional<PlaybackFailure> failure;
};

class PlaybackSnapshot final
{
public:
    PlaybackSnapshot() = default;
    explicit PlaybackSnapshot(PlaybackSnapshotState state);

    [[nodiscard]] static PlaybackSnapshot opening(MediaGeneration generation, QString source);
    [[nodiscard]] static PlaybackSnapshot stopped(MediaGeneration generation = {});

    [[nodiscard]] const PlaybackSnapshotState& state() const noexcept;
    [[nodiscard]] MediaGeneration generation() const noexcept;
    [[nodiscard]] PlaybackLifecycleState lifecycle() const noexcept;
    [[nodiscard]] PlaybackTransportState transport() const noexcept;
    [[nodiscard]] const PlaybackMediaState& media() const noexcept;
    [[nodiscard]] const PlaybackTimelineState& timeline() const noexcept;
    [[nodiscard]] const PlaybackBufferingState& buffering() const noexcept;
    [[nodiscard]] const PlaybackControlsState& controls() const noexcept;
    [[nodiscard]] const PlaybackCapabilitiesState& capabilities() const noexcept;
    [[nodiscard]] const PlaybackStreamState& streams() const noexcept;
    [[nodiscard]] const PlaybackTrackState& tracks() const noexcept;
    [[nodiscard]] const PlaybackChapterState& chapters() const noexcept;
    [[nodiscard]] const std::optional<PlaybackFailure>& failure() const noexcept;

private:
    PlaybackSnapshotState state_;
};

} // namespace player::playback::domain

Q_DECLARE_METATYPE(player::playback::domain::PlaybackSnapshot)
