#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QtGlobal>

namespace player::presentation {

enum class PlayerStatusKind : quint8
{
    None,
    Loading,
    Buffering,
    Ended,
    Error,
};

[[nodiscard]] PlayerStatusKind selectPlayerStatus(
    const player::playback::domain::PlaybackSnapshot& snapshot) noexcept;

} // namespace player::presentation
