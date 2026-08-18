#pragma once

#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/transport_command.h"
#include "playback/domain/models/track_descriptor.h"

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

namespace player::tracks::application {
class AudioDelayController;
class ExternalSubtitleLoader;
class SubtitleDelayController;
class TrackSelectionController;
}

namespace player::tracks::presentation {
class ChapterListModel;
class TrackListModel;
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
    [[nodiscard]] player::tracks::application::TrackSelectionController& trackSelectionController() noexcept;
    [[nodiscard]] player::tracks::application::SubtitleDelayController& subtitleDelayController() noexcept;
    [[nodiscard]] player::tracks::application::AudioDelayController& audioDelayController() noexcept;
    [[nodiscard]] player::tracks::application::ExternalSubtitleLoader& externalSubtitleLoader() noexcept;
    [[nodiscard]] player::tracks::presentation::TrackListModel& audioTrackListModel() noexcept;
    [[nodiscard]] player::tracks::presentation::TrackListModel& subtitleTrackListModel() noexcept;
    [[nodiscard]] player::tracks::presentation::TrackListModel& videoTrackListModel() noexcept;
    [[nodiscard]] player::tracks::presentation::ChapterListModel& chapterListModel() noexcept;

    void setPlaybackSupersessionObserver(PlaybackSupersessionObserver observer);
    [[nodiscard]] bool submitMediaLoad(const QString& canonicalSource);
    [[nodiscard]] bool submitMediaStop();

private:
    [[nodiscard]] bool submitTransport(player::playback::domain::TransportAction action);
    [[nodiscard]] bool submitSeek(
        double seconds,
        player::playback::domain::SeekMode mode);
    [[nodiscard]] bool submitVolume(double percent);
    [[nodiscard]] bool submitMuted(bool muted);
    [[nodiscard]] bool submitTrackSelection(
        player::playback::domain::TrackKind kind,
        qint64 trackId);
    [[nodiscard]] bool submitSubtitleDelay(double seconds);
    [[nodiscard]] bool submitAudioDelay(double seconds);
    [[nodiscard]] bool submitExternalSubtitle(const QString& path);
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
    std::unique_ptr<player::tracks::application::TrackSelectionController> trackSelectionController_;
    std::unique_ptr<player::tracks::application::SubtitleDelayController> subtitleDelayController_;
    std::unique_ptr<player::tracks::application::AudioDelayController> audioDelayController_;
    std::unique_ptr<player::tracks::application::ExternalSubtitleLoader> externalSubtitleLoader_;
    std::unique_ptr<player::tracks::presentation::TrackListModel> audioTrackListModel_;
    std::unique_ptr<player::tracks::presentation::TrackListModel> subtitleTrackListModel_;
    std::unique_ptr<player::tracks::presentation::TrackListModel> videoTrackListModel_;
    std::unique_ptr<player::tracks::presentation::ChapterListModel> chapterListModel_;
    PlaybackSupersessionObserver playbackSupersessionObserver_;
};

} // namespace player::app
