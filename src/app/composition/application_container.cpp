#include "app/composition/application_container.h"

#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/qml_bootstrap.h"

#include <utility>

namespace player::app {

ApplicationContainer::ApplicationContainer(RuntimePaths runtimePaths)
    : runtimePaths_(std::move(runtimePaths))
    , loggingBootstrap_(std::make_unique<LoggingBootstrap>())
    , qmlBootstrap_(std::make_unique<QmlBootstrap>())
{
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

QmlBootstrap& ApplicationContainer::qmlBootstrap() noexcept
{
    return *qmlBootstrap_;
}

void ApplicationContainer::shutdown() noexcept
{
    // QML owns the current object tree. Destroy it while logging is still alive
    // so shutdown diagnostics remain available, then stop and release logging.
    qmlBootstrap_.reset();

    if (loggingBootstrap_ != nullptr) {
        loggingBootstrap_->stop();
        loggingBootstrap_.reset();
    }
}

} // namespace player::app
