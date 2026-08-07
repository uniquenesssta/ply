#include "app/bootstrap/application_bootstrap.h"

#include "app/bootstrap/graphics_backend/graphics_backend_probe.h"
#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/qml_bootstrap.h"
#include "app/bootstrap/runtime_paths.h"
#include "app/composition/application_container.h"
#include "foundation/logging/log_categories.h"
#include "playback/infrastructure/mpv/runtime/mpv_runtime_probe.h"

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QString>

#include <cstdlib>
#include <utility>

namespace player::app {

int ApplicationBootstrap::run(QGuiApplication& application)
{
    configureApplicationMetadata(application);

    RuntimePaths runtimePaths = RuntimePaths::current();
    ApplicationContainer container(std::move(runtimePaths));

    QString loggingError;
    if (!container.loggingBootstrap().start(container.runtimePaths(), &loggingError)) {
        qWarning().noquote() << "File logging is unavailable:" << loggingError;
    }

    qCInfo(player::logging::appLifecycle) << "Application starting";

    player::playback::mpv::MpvRuntimeInfo mpvRuntimeInfo;
    QString mpvRuntimeError;
    if (!player::playback::mpv::MpvRuntimeProbe::probe(mpvRuntimeInfo, &mpvRuntimeError)) {
        qCCritical(player::logging::playbackMpv).noquote()
            << "libmpv runtime validation failed:" << mpvRuntimeError;
        return EXIT_FAILURE;
    }

    qCInfo(player::logging::playbackMpv).noquote()
        << "libmpv runtime validated:"
        << "mpv=" << mpvRuntimeInfo.manifest.mpvVersion
        << "tag=" << mpvRuntimeInfo.manifest.mpvTag
        << "commit=" << mpvRuntimeInfo.manifest.mpvCommit
        << "ffmpeg=" << mpvRuntimeInfo.manifest.ffmpegVersion
        << "clientApi=" << QStringLiteral("%1.%2")
                               .arg(mpvRuntimeInfo.clientApiMajor)
                               .arg(mpvRuntimeInfo.clientApiMinor)
        << "dll=" << mpvRuntimeInfo.runtimeLibraryPath
        << "manifest=" << mpvRuntimeInfo.manifestPath;

    GraphicsBackendInfo graphicsInfo;
    QString graphicsError;
    if (!GraphicsBackendProbe::probe(graphicsInfo, &graphicsError)) {
        qCCritical(player::logging::appBootstrap).noquote()
            << "OpenGL backend validation failed:" << graphicsError;
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

    QmlBootstrap& qmlBootstrap = container.qmlBootstrap();
    if (!qmlBootstrap.load()) {
        qCCritical(player::logging::appBootstrap).noquote()
            << "QML bootstrap failed:" << qmlBootstrap.lastError();
        return EXIT_FAILURE;
    }

    const int exitCode = application.exec();

    qCInfo(player::logging::appLifecycle) << "Application stopping with exit code" << exitCode;
    container.shutdown();
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
