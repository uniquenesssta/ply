#include "app/bootstrap/application_bootstrap.h"
#include "app/bootstrap/graphics_backend/graphics_backend_bootstrap.h"
#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/runtime_paths.h"
#include "foundation/logging/log_categories.h"

#include <QDebug>
#include <QGuiApplication>
#include <QString>

#include <memory>
#include <utility>

int main(int argc, char* argv[])
{
    player::app::GraphicsBackendBootstrap::configure();
    player::app::ApplicationBootstrap::configureApplicationMetadata();

    player::app::RuntimePaths runtimePaths =
        player::app::RuntimePaths::fromCurrentProcessExecutable();

    auto loggingBootstrap = std::make_unique<player::app::LoggingBootstrap>();
    QString loggingError;
    if (!loggingBootstrap->start(runtimePaths, &loggingError)) {
        qWarning().noquote() << "Pre-GUI file logging is unavailable:" << loggingError;
    }

    qCInfo(player::logging::appLifecycle) << "Pre-GUI bootstrap starting";

    QGuiApplication application(argc, argv);
    player::app::ApplicationBootstrap bootstrap;

    return bootstrap.run(
        application,
        std::move(runtimePaths),
        std::move(loggingBootstrap));
}
