#include "playback_shutdown.h"

#include "backend/playback_session_backend.h"
#include "playback/application/requests/request_timeout_monitor.h"
#include "playback/application/requests/request_tracker.h"

namespace player::playback::application {

void executePlaybackShutdown(
    RequestTracker& requestTracker,
    RequestTimeoutMonitor* requestTimeoutMonitor,
    PlaybackSessionBackend& backend) noexcept
{
    if (requestTimeoutMonitor != nullptr) {
        requestTimeoutMonitor->stop();
    }

    (void)requestTracker.cancelAll(PlaybackRequestCancellationReason::Shutdown);
    backend.shutdown();
}

} // namespace player::playback::application
