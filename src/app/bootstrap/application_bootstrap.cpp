#include "app/bootstrap/application_bootstrap.h"

#include "app/bootstrap/graphics_backend/graphics_backend_probe.h"
#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/qml_bootstrap.h"
#include "app/bootstrap/startup_media_open_scheduler.h"
#include "app/composition/application_container.h"
#include "app/composition/playback_composition.h"
#include "foundation/logging/log_categories.h"
#include "media/application/drop/media_drop_handler.h"
#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"
#include "playback/infrastructure/mpv/runtime/mpv_runtime_probe.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/presentation/playlist_list_model.h"
#include "presentation/qml/types/presentation_type_registration.h"
#include "presentation/viewmodels/player/hud/hud_message_queue.h"
#include "presentation/viewmodels/player/media/player_media_view_model.h"
#include "presentation/viewmodels/player/status/player_status_view_model.h"
#include "presentation/viewmodels/player/timeline/player_timeline_view_model.h"
#include "presentation/viewmodels/player/transport/player_transport_view_model.h"
#include "presentation/viewmodels/player/volume/player_volume_view_model.h"
#include "tracks/application/audio_delay_controller.h"
#include "tracks/application/external_subtitle_loader.h"
#include "tracks/application/subtitle_delay_controller.h"
#include "tracks/application/track_selection_controller.h"
#include "tracks/presentation/track_list_model.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QObject>
#include <QQuickWindow>
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
        QStringLiteral("mediaViewModel"),
        QVariant::fromValue(
            static_cast<QObject*>(&playbackComposition.mediaViewModel())));
    initialProperties.insert(
        QStringLiteral("hudMessageQueue"),
        QVariant::fromValue(
            static_cast<QObject*>(&playbackComposition.hudMessageQueue())));
    initialProperties.insert(
        QStringLiteral("playlistController"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.playlistController())));
    initialProperties.insert(
        QStringLiteral("playlistModel"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.playlistListModel())));
    initialProperties.insert(
        QStringLiteral("trackSelectionController"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.trackSelectionController())));
    initialProperties.insert(
        QStringLiteral("externalSubtitleLoader"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.externalSubtitleLoader())));
    initialProperties.insert(
        QStringLiteral("subtitleDelayController"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.subtitleDelayController())));
    initialProperties.insert(
        QStringLiteral("audioDelayController"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.audioDelayController())));
    initialProperties.insert(
        QStringLiteral("audioTrackModel"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.audioTrackListModel())));
    initialProperties.insert(
        QStringLiteral("subtitleTrackModel"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.subtitleTrackListModel())));
    initialProperties.insert(
        QStringLiteral("mediaOpenCoordinator"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.mediaOpenCoordinator())));
    initialProperties.insert(
        QStringLiteral("urlOpenWorkflow"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.urlOpenWorkflow())));
    initialProperties.insert(
        QStringLiteral("mediaDropHandler"),
        QVariant::fromValue(
            static_cast<QObject*>(&container.mediaDropHandler())));
    qmlBootstrap.setInitialProperties(initialProperties);

    if (!qmlBootstrap.load()) {
        qCCritical(player::logging::appBootstrap).noquote()
            << "QML bootstrap failed:" << qmlBootstrap.lastError();
        return EXIT_FAILURE;
    }

    QString videoOutputError;
    if (!playbackComposition.attachVideoOutput(
            qmlBootstrap.rootObject(),
            &videoOutputError)) {
        qCCritical(player::logging::appBootstrap).noquote()
            << "Video output binding failed:" << videoOutputError;
        return EXIT_FAILURE;
    }

    QString startupMediaError;
    if (!StartupMediaOpenScheduler::scheduleAfterFirstFrame(
            qobject_cast<QQuickWindow*>(qmlBootstrap.rootObject()),
            container.mediaArgumentOpenWorkflow(),
            QCoreApplication::arguments(),
            QDir::currentPath(),
            &startupMediaError)) {
        qCCritical(player::logging::appBootstrap).noquote()
            << "Startup media scheduling failed:" << startupMediaError;
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
