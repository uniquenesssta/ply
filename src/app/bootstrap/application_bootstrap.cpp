#include "app/bootstrap/application_bootstrap.h"

#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/qml_bootstrap.h"
#include "app/bootstrap/runtime_paths.h"
#include "foundation/logging/log_categories.h"

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QString>

#include <cstdlib>

namespace player::app {

int ApplicationBootstrap::run(QGuiApplication& application)
{
    configureApplicationMetadata(application);

    const RuntimePaths runtimePaths = RuntimePaths::current();

    LoggingBootstrap loggingBootstrap;
    QString loggingError;
    if (!loggingBootstrap.start(runtimePaths, &loggingError)) {
        qWarning().noquote() << "File logging is unavailable:" << loggingError;
    }

    qCInfo(player::logging::appLifecycle) << "Application starting";

    QmlBootstrap qmlBootstrap;
    if (!qmlBootstrap.load()) {
        qCCritical(player::logging::appBootstrap) << "QML bootstrap failed";
        loggingBootstrap.stop();
        return EXIT_FAILURE;
    }

    const int exitCode = application.exec();

    qCInfo(player::logging::appLifecycle) << "Application stopping with exit code" << exitCode;
    loggingBootstrap.stop();
    return exitCode;
}

void ApplicationBootstrap::configureApplicationMetadata(QGuiApplication& application)
{
    application.setOrganizationName(QStringLiteral("ModularPlayer"));
    application.setOrganizationDomain(QStringLiteral("local.modularplayer"));
    application.setApplicationName(QStringLiteral("Player"));
    application.setApplicationVersion(QStringLiteral("0.1.0"));
}

} // namespace player::app
