#pragma once

#include "playback/domain/errors/playback_failure.h"

namespace player::playback::domain {

struct MediaFailedEvent final
{
    PlaybackFailure failure;
};

struct PlaybackFailureEvent final
{
    PlaybackFailure failure;
};

} // namespace player::playback::domain
