#pragma once

#include "foundation/ids/request_id.h"

#include <QtGlobal>

#include <atomic>

namespace player::playback::application {

class PlaybackRequestIdGenerator final
{
public:
    [[nodiscard]] player::ids::RequestId next() noexcept;

private:
    std::atomic<quint64> nextValue_{1};
};

} // namespace player::playback::application
