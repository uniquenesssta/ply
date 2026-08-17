#pragma once

#include "app/bootstrap/runtime_paths.h"

#include <memory>

namespace player::media::application {
class MediaArgumentOpenWorkflow;
class MediaDropHandler;
class MediaOpenCoordinator;
class UrlOpenWorkflow;
}

namespace player::playlist::application {
class PlaylistAutoAdvance;
class PlaylistController;
class PlaylistMutation;
}

namespace player::playlist::domain {
class Playlist;
}

namespace player::playlist::presentation {
class PlaylistListModel;
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
    std::unique_ptr<player::playlist::application::PlaylistController> playlistController_;
    std::unique_ptr<player::playlist::application::PlaylistAutoAdvance> playlistAutoAdvance_;
    std::unique_ptr<player::playlist::presentation::PlaylistListModel> playlistListModel_;
    std::unique_ptr<player::media::application::MediaOpenCoordinator> mediaOpenCoordinator_;
    std::unique_ptr<player::media::application::UrlOpenWorkflow> urlOpenWorkflow_;
    std::unique_ptr<player::media::application::MediaArgumentOpenWorkflow> mediaArgumentOpenWorkflow_;
    std::unique_ptr<player::media::application::MediaDropHandler> mediaDropHandler_;
    std::unique_ptr<QmlBootstrap> qmlBootstrap_;
};

} // namespace player::app
