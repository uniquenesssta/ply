#pragma once

#include "foundation/ids/request_id.h"
#include "playback/domain/events/failure_event.h"

#include <optional>

namespace player::playback::domain {

struct CommandReplyEvent final
{
    player::ids::RequestId requestId;
    bool succeeded = false;
    std::optional<PlaybackFailure> failure;
};

} // namespace player::playback::domain
