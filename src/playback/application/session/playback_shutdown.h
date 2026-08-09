#pragma once

namespace player::playback::application {

class PlaybackSessionBackend;
class RequestTimeoutMonitor;
class RequestTracker;

void executePlaybackShutdown(
    RequestTracker& requestTracker,
    RequestTimeoutMonitor* requestTimeoutMonitor,
    PlaybackSessionBackend& backend) noexcept;

} // namespace player::playback::application
