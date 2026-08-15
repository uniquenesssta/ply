#pragma once

#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/transport_command.h"

#include <memory>

class QString;

namespace player::playback::application {
class PlaybackRequestIdGenerator;
class PlaybackSessionThread;
}

namespace player::presentation {
class HudMessageQueue;
class PlayerStatusViewModel;
class PlayerTimelineViewModel;
class PlayerTransportViewModel;
class PlayerVolumeViewModel;
}

namespace player::app {

class PlaybackComposition final
{
public:
    PlaybackComposition();
    ~PlaybackComposition();

    PlaybackComposition(const PlaybackComposition&) = delete;
    PlaybackComposition& operator=(const PlaybackComposition&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);
    [[nodiscard]] bool stop(QString* errorMessage = nullptr);
    [[nodiscard]] bool isRunning() const noexcept;

    [[nodiscard]] player::presentation::PlayerTransportViewModel& transportViewModel() noexcept;
    [[nodiscard]] player::presentation::PlayerTimelineViewModel& timelineViewModel() noexcept;
    [[nodiscard]] player::presentation::PlayerVolumeViewModel& volumeViewModel() noexcept;
    [[nodiscard]] player::presentation::PlayerStatusViewModel& statusViewModel() noexcept;
    [[nodiscard]] player::presentation::HudMessageQueue& hudMessageQueue() noexcept;

private:
    void submitTransport(player::playback::domain::TransportAction action);
    [[nodiscard]] bool submitSeek(
        double seconds,
        player::playback::domain::SeekMode mode);
    [[nodiscard]] bool submitVolume(double percent);
    [[nodiscard]] bool submitMuted(bool muted);

    std::unique_ptr<player::playback::application::PlaybackSessionThread> playbackThread_;
    std::unique_ptr<player::playback::application::PlaybackRequestIdGenerator> requestIdGenerator_;
    std::unique_ptr<player::presentation::PlayerTransportViewModel> transportViewModel_;
    std::unique_ptr<player::presentation::PlayerTimelineViewModel> timelineViewModel_;
    std::unique_ptr<player::presentation::PlayerVolumeViewModel> volumeViewModel_;
    std::unique_ptr<player::presentation::PlayerStatusViewModel> statusViewModel_;
    std::unique_ptr<player::presentation::HudMessageQueue> hudMessageQueue_;
};

} // namespace player::app
