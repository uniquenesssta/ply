#include "app/composition/application_container.h"

#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/qml_bootstrap.h"
#include "app/composition/playback_composition.h"
#include "foundation/logging/log_categories.h"
#include "media/application/drop/media_drop_handler.h"
#include "media/application/open/media_open_coordinator.h"

#include <QLoggingCategory>
#include <QString>

#include <utility>

namespace player::app {

ApplicationContainer::ApplicationContainer(RuntimePaths runtimePaths)
    : ApplicationContainer(
        std::move(runtimePaths),
        std::make_unique<LoggingBootstrap>())
{
}

ApplicationContainer::ApplicationContainer(
    RuntimePaths runtimePaths,
    std::unique_ptr<LoggingBootstrap> loggingBootstrap)
    : runtimePaths_(std::move(runtimePaths))
    , loggingBootstrap_(std::move(loggingBootstrap))
    , playbackComposition_(std::make_unique<PlaybackComposition>())
    , mediaOpenCoordinator_(
        std::make_unique<player::media::application::MediaOpenCoordinator>(
            [this](const player::media::domain::MediaSource& source) {
                return playbackComposition_ != nullptr
                    && playbackComposition_->submitMediaLoad(source.location());
            }))
    , mediaDropHandler_(
        std::make_unique<player::media::application::MediaDropHandler>(
            *mediaOpenCoordinator_))
    , qmlBootstrap_(std::make_unique<QmlBootstrap>())
{
    if (loggingBootstrap_ == nullptr) {
        loggingBootstrap_ = std::make_unique<LoggingBootstrap>();
    }
}

ApplicationContainer::~ApplicationContainer()
{
    shutdown();
}

const RuntimePaths& ApplicationContainer::runtimePaths() const noexcept
{
    return runtimePaths_;
}

LoggingBootstrap& ApplicationContainer::loggingBootstrap() noexcept
{
    return *loggingBootstrap_;
}

PlaybackComposition& ApplicationContainer::playbackComposition() noexcept
{
    return *playbackComposition_;
}

player::media::application::MediaOpenCoordinator&
ApplicationContainer::mediaOpenCoordinator() noexcept
{
    return *mediaOpenCoordinator_;
}

player::media::application::MediaDropHandler&
ApplicationContainer::mediaDropHandler() noexcept
{
    return *mediaDropHandler_;
}

QmlBootstrap& ApplicationContainer::qmlBootstrap() noexcept
{
    return *qmlBootstrap_;
}

void ApplicationContainer::shutdown() noexcept
{
    // Freeze the R4 render callback before destroying QML, then destroy the
    // video item/scene graph while the playback-owned mpv core is still alive.
    // PlaybackComposition::stop() subsequently proves render release before it
    // is allowed to stop PlaybackSession and destroy that core.
    if (playbackComposition_ != nullptr) {
        playbackComposition_->beginVideoRenderShutdown();
    }

    qmlBootstrap_.reset();
    mediaDropHandler_.reset();
    mediaOpenCoordinator_.reset();

    if (playbackComposition_ != nullptr) {
        QString diagnostic;
        if (!playbackComposition_->stop(&diagnostic)) {
            qCCritical(player::logging::appLifecycle).noquote()
                << "Playback composition shutdown failed:" << diagnostic;
        }
        playbackComposition_.reset();
    }

    if (loggingBootstrap_ != nullptr) {
        loggingBootstrap_->stop();
        loggingBootstrap_.reset();
    }
}

} // namespace player::app
