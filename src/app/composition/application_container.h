#pragma once

#include "app/bootstrap/runtime_paths.h"

#include <memory>

namespace player::media::application {
class MediaOpenCoordinator;
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
    [[nodiscard]] player::media::application::MediaOpenCoordinator& mediaOpenCoordinator() noexcept;
    [[nodiscard]] QmlBootstrap& qmlBootstrap() noexcept;

    void shutdown() noexcept;

private:
    RuntimePaths runtimePaths_;
    std::unique_ptr<LoggingBootstrap> loggingBootstrap_;
    std::unique_ptr<PlaybackComposition> playbackComposition_;
    std::unique_ptr<player::media::application::MediaOpenCoordinator> mediaOpenCoordinator_;
    std::unique_ptr<QmlBootstrap> qmlBootstrap_;
};

} // namespace player::app
