#pragma once

#include "app/bootstrap/runtime_paths.h"

#include <memory>

namespace player::chapters::presentation {
class ChapterModel;
}

namespace player::media::application {
class MediaArgumentOpenWorkflow;
class MediaDropHandler;
class MediaOpenCoordinator;
class UrlOpenWorkflow;
}

namespace player::playlist::application {
class PlaylistAdvanceArbiter;
class PlaylistAutoAdvance;
class PlaylistController;
class PlaylistMutation;
}

namespace player::playlist::domain {
class Playlist;
}

namespace player::playlist::presentation {
class PlaylistEntryPlaybackState;
class PlaylistListModel;
}

namespace player::tracks::application {
class AudioDelayController;
class ExternalSubtitleLoader;
class SubtitleDelayController;
class TrackSelectionController;
}

namespace player::tracks::presentation {
class TrackListModel;
}

namespace player::app {

class LoggingBootstrap;
class PlaybackComposition;
class QmlBootstrap;

class ApplicationContainer final
{
public:
    explicit ApplicationContainer(RuntimePaths runtimePaths);
    ApplicationContainer(
        RuntimePaths runtimePaths,
        std::unique_ptr<LoggingBootstrap> loggingBootstrap);
    ~ApplicationContainer();

    ApplicationContainer(const ApplicationContainer&) = delete;
    ApplicationContainer& operator=(const ApplicationContainer&) = delete;
    ApplicationContainer(ApplicationContainer&&) = delete;
    ApplicationContainer& operator=(ApplicationContainer&&) = delete;

    [[nodiscard]] const RuntimePaths& runtimePaths() const noexcept;
    [[nodiscard]] LoggingBootstrap& loggingBootstrap() noexcept;
    [[nodiscard]] PlaybackComposition& playbackComposition() noexcept;
    [[nodiscard]] player::playlist::application::PlaylistController& playlistController() noexcept;
    [[nodiscard]] player::playlist::presentation::PlaylistListModel& playlistListModel() noexcept;
    [[nodiscard]] player::tracks::application::TrackSelectionController& trackSelectionController() noexcept;
    [[nodiscard]] player::tracks::application::ExternalSubtitleLoader& externalSubtitleLoader() noexcept;
    [[nodiscard]] player::tracks::application::SubtitleDelayController& subtitleDelayController() noexcept;
    [[nodiscard]] player::tracks::application::AudioDelayController& audioDelayController() noexcept;
    [[nodiscard]] player::tracks::presentation::TrackListModel& audioTrackListModel() noexcept;
    [[nodiscard]] player::tracks::presentation::TrackListModel& subtitleTrackListModel() noexcept;
    [[nodiscard]] player::chapters::presentation::ChapterModel& chapterModel() noexcept;
    [[nodiscard]] player::media::application::MediaOpenCoordinator& mediaOpenCoordinator() noexcept;
    [[nodiscard]] player::media::application::UrlOpenWorkflow& urlOpenWorkflow() noexcept;
    [[nodiscard]] player::media::application::MediaArgumentOpenWorkflow& mediaArgumentOpenWorkflow() noexcept;
    [[nodiscard]] player::media::application::MediaDropHandler& mediaDropHandler() noexcept;
    [[nodiscard]] QmlBootstrap& qmlBootstrap() noexcept;

    void shutdown() noexcept;

private:
    RuntimePaths runtimePaths_;
    std::unique_ptr<LoggingBootstrap> loggingBootstrap_;
    std::unique_ptr<PlaybackComposition> playbackComposition_;
    std::unique_ptr<player::playlist::domain::Playlist> playlist_;
    std::unique_ptr<player::playlist::application::PlaylistMutation> playlistMutation_;
    std::unique_ptr<player::playlist::application::PlaylistAdvanceArbiter> playlistAdvanceArbiter_;
    std::unique_ptr<player::playlist::application::PlaylistController> playlistController_;
    std::unique_ptr<player::playlist::presentation::PlaylistEntryPlaybackState> playlistEntryPlaybackState_;
    std::unique_ptr<player::playlist::application::PlaylistAutoAdvance> playlistAutoAdvance_;
    std::unique_ptr<player::playlist::presentation::PlaylistListModel> playlistListModel_;
    std::unique_ptr<player::tracks::application::TrackSelectionController> trackSelectionController_;
    std::unique_ptr<player::tracks::application::ExternalSubtitleLoader> externalSubtitleLoader_;
    std::unique_ptr<player::tracks::application::SubtitleDelayController> subtitleDelayController_;
    std::unique_ptr<player::tracks::application::AudioDelayController> audioDelayController_;
    std::unique_ptr<player::tracks::presentation::TrackListModel> audioTrackListModel_;
    std::unique_ptr<player::tracks::presentation::TrackListModel> subtitleTrackListModel_;
    std::unique_ptr<player::chapters::presentation::ChapterModel> chapterModel_;
    std::unique_ptr<player::media::application::MediaOpenCoordinator> mediaOpenCoordinator_;
    std::unique_ptr<player::media::application::UrlOpenWorkflow> urlOpenWorkflow_;
    std::unique_ptr<player::media::application::MediaArgumentOpenWorkflow> mediaArgumentOpenWorkflow_;
    std::unique_ptr<player::media::application::MediaDropHandler> mediaDropHandler_;
    std::unique_ptr<QmlBootstrap> qmlBootstrap_;
};

} // namespace player::app
