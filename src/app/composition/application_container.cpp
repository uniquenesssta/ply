#include "app/composition/application_container.h"

#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/qml_bootstrap.h"
#include "app/composition/playback_composition.h"
#include "foundation/logging/log_categories.h"

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

QmlBootstrap& ApplicationContainer::qmlBootstrap() noexcept
{
    return *qmlBootstrap_;
}

void ApplicationContainer::shutdown() noexcept
{
    // Destroy UI intents first, then stop the playback thread while logging is
    // still alive so any bounded-shutdown diagnostics remain observable.
    qmlBootstrap_.reset();

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
