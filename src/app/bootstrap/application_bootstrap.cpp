#include "app/bootstrap/application_bootstrap.h"

#include "app/bootstrap/graphics_backend/graphics_backend_probe.h"
#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/qml_bootstrap.h"
#include "app/composition/application_container.h"
#include "app/composition/playback_composition.h"
#include "foundation/logging/log_categories.h"
#include "playback/infrastructure/mpv/runtime/mpv_runtime_probe.h"
#include "presentation/qml/types/presentation_type_registration.h"
#include "presentation/viewmodels/player/hud/hud_message_queue.h"
#include "presentation/viewmodels/player/status/player_status_view_model.h"
#include "presentation/viewmodels/player/timeline/player_timeline_view_model.h"
#include "presentation/viewmodels/player/transport/player_transport_view_model.h"
#include "presentation/viewmodels/player/volume/player_volume_view_model.h"

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

#include <cstdlib>
#include <memory>
#include <utility>

namespace player::app {

int ApplicationBootstrap::run(
    QGuiApplication& application,
    RuntimePaths runtimePaths,
    std::unique_ptr<LoggingBootstrap> loggingBootstrap)
{
    ApplicationContainer container(
        std::move(runtimePaths),
        std::move(loggingBootstrap));

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

    if (!player::presentation::qml::registerPresentationQmlTypes()) {
        qCCritical(player::logging::appBootstrap)
            << "Failed to register Player.Presentation C++ QML types.";
        return EXIT_FAILURE;
    }

    PlaybackComposition& playbackComposition = container.playbackComposition();
    QString playbackError;
    if (!playbackComposition.start(&playbackError)) {
        qCCritical(player::logging::appLifecycle).noquote()
            << "Playback composition failed to start:" << playbackError;
        return EXIT_FAILURE;
    }

    QmlBootstrap& qmlBootstrap = container.qmlBootstrap();
    QVariantMap initialProperties;
    initialProperties.insert(
        QStringLiteral("transportViewModel"),
        QVariant::fromValue(
            static_cast<QObject*>(&playbackComposition.transportViewModel())));
    initialProperties.insert(
        QStringLiteral("timelineViewModel"),
        QVariant::fromValue(
            static_cast<QObject*>(&playbackComposition.timelineViewModel())));
    initialProperties.insert(
        QStringLiteral("volumeViewModel"),
        QVariant::fromValue(
            static_cast<QObject*>(&playbackComposition.volumeViewModel())));
    initialProperties.insert(
        QStringLiteral("statusViewModel"),
        QVariant::fromValue(
            static_cast<QObject*>(&playbackComposition.statusViewModel())));
    initialProperties.insert(
        QStringLiteral("hudMessageQueue"),
        QVariant::fromValue(
            static_cast<QObject*>(&playbackComposition.hudMessageQueue())));
    qmlBootstrap.setInitialProperties(initialProperties);

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

void ApplicationBootstrap::configureApplicationMetadata()
{
    QCoreApplication::setOrganizationName(QStringLiteral("ModularPlayer"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("local.modularplayer"));
    QCoreApplication::setApplicationName(QStringLiteral("Player"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
}

} // namespace player::app
