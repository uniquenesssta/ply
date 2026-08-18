#pragma once

#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/track_selection_command.h"
#include "playback/domain/commands/transport_command.h"

#include <functional>
#include <memory>

class QObject;
class QString;

namespace player::playback::application {
class PlaybackRequestIdGenerator;
class PlaybackSessionThread;
class StatePublisher;
}

namespace player::presentation {
class HudMessageQueue;
class PlayerMediaViewModel;
class PlayerStatusViewModel;
class PlayerTimelineViewModel;
class PlayerTransportViewModel;
class PlayerVolumeViewModel;
}

namespace player::app {

class PlayerVideoRenderBinding;

class PlaybackComposition final
{
public:
    using PlaybackSupersessionObserver = std::function<void()>;

    PlaybackComposition();
    ~PlaybackComposition();

    PlaybackComposition(const PlaybackComposition&) = delete;
    PlaybackComposition& operator=(const PlaybackComposition&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);
    [[nodiscard]] bool stop(QString* errorMessage = nullptr);
    [[nodiscard]] bool isRunning() const noexcept;

    [[nodiscard]] bool attachVideoOutput(QObject* qmlRoot, QString* errorMessage = nullptr);
    void beginVideoRenderShutdown() noexcept;

    [[nodiscard]] player::playback::application::StatePublisher& statePublisher() noexcept;
    [[nodiscard]] player::presentation::PlayerTransportViewModel& transportViewModel() noexcept;
    [[nodiscard]] player::presentation::PlayerTimelineViewModel& timelineViewModel() noexcept;
    [[nodiscard]] player::presentation::PlayerVolumeViewModel& volumeViewModel() noexcept;
    [[nodiscard]] player::presentation::PlayerStatusViewModel& statusViewModel() noexcept;
    [[nodiscard]] player::presentation::PlayerMediaViewModel& mediaViewModel() noexcept;
    [[nodiscard]] player::presentation::HudMessageQueue& hudMessageQueue() noexcept;

    void setPlaybackSupersessionObserver(PlaybackSupersessionObserver observer);
    [[nodiscard]] bool submitMediaLoad(const QString& canonicalSource);
    [[nodiscard]] bool submitMediaStop();
    [[nodiscard]] bool submitTrackSelection(
        const player::playback::domain::TrackSelectionCommand& selection);

private:
    [[nodiscard]] bool submitTransport(player::playback::domain::TransportAction action);
    [[nodiscard]] bool submitSeek(
        double seconds,
        player::playback::domain::SeekMode mode);
    [[nodiscard]] bool submitVolume(double percent);
    [[nodiscard]] bool submitMuted(bool muted);
    void notifyPlaybackSupersessionAccepted();

    std::unique_ptr<player::playback::application::PlaybackSessionThread> playbackThread_;
    std::unique_ptr<PlayerVideoRenderBinding> videoRenderBinding_;
    std::unique_ptr<player::playback::application::PlaybackRequestIdGenerator> requestIdGenerator_;
    std::unique_ptr<player::presentation::PlayerTransportViewModel> transportViewModel_;
    std::unique_ptr<player::presentation::PlayerTimelineViewModel> timelineViewModel_;
    std::unique_ptr<player::presentation::PlayerVolumeViewModel> volumeViewModel_;
    std::unique_ptr<player::presentation::PlayerStatusViewModel> statusViewModel_;
    std::unique_ptr<player::presentation::PlayerMediaViewModel> mediaViewModel_;
    std::unique_ptr<player::presentation::HudMessageQueue> hudMessageQueue_;
    PlaybackSupersessionObserver playbackSupersessionObserver_;
};

} // namespace player::app
