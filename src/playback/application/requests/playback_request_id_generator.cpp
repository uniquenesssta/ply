#include "playback/application/requests/playback_request_id_generator.h"

#include <atomic>

namespace player::playback::application {

player::ids::RequestId PlaybackRequestIdGenerator::next() noexcept
{
    for (;;) {
        const quint64 value = nextValue_.fetch_add(1, std::memory_order_relaxed);
        if (value != 0) {
            return player::ids::RequestId{value};
        }
    }
}

} // namespace player::playback::application
