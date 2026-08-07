#include "app/bootstrap/application_bootstrap.h"

#include "app/bootstrap/graphics_backend/graphics_backend_probe.h"
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

    GraphicsBackendInfo graphicsInfo;
    QString graphicsError;
    if (!GraphicsBackendProbe::probe(graphicsInfo, &graphicsError)) {
        qCCritical(player::logging::appBootstrap).noquote()
            << "OpenGL backend validation failed:" << graphicsError;
        loggingBootstrap.stop();
        return EXIT_FAILURE;
    }

    qCInfo(player::logging::appBootstrap).noquote()
        << "Graphics backend validated:"
        << "api=OpenGL"
        << "profile=" << (graphicsInfo.isOpenGles ? "OpenGL ES" : "Desktop OpenGL")
        << "context=" << QStringLiteral("%1.%2").arg(graphicsInfo.majorVersion).arg(graphicsInfo.minorVersion)
        << "vendor=" << graphicsInfo.vendor
        << "renderer=" << graphicsInfo.renderer
        << "version=" << graphicsInfo.version
        << "glsl=" << graphicsInfo.shadingLanguageVersion;

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
