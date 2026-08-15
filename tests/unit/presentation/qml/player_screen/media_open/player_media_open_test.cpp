#include <QFile>
#include <QString>
#include <QtTest>

namespace player::presentation::qml {
namespace {

QString readSource(const QString& relativePath)
{
    QFile file(QStringLiteral(PLAYER_SOURCE_DIR "/") + relativePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace

class PlayerMediaOpenTest final : public QObject
{
    Q_OBJECT

private slots:
    void nativePickerOnlyPublishesAcceptedLocalSelection();
    void mainWindowRoutesIntentThroughCoordinator();
    void playbackScreenAndOverlayRemainPresentationOnly();
    void bootstrapInjectsSingleCoordinatorBoundary();
    void mediaCapabilityUsesSnapshotProjection();
    void localValidationAndCoordinationStaySeparated();
};

void PlayerMediaOpenTest::nativePickerOnlyPublishesAcceptedLocalSelection()
{
    const QString source = readSource(QStringLiteral(
        "src/presentation/qml/features/player/mediaopen/LocalMediaOpenDialog.qml"));
    const QString presentationCmake = readSource(QStringLiteral(
        "src/presentation/CMakeLists.txt"));
    QVERIFY(!source.isEmpty());
    QVERIFY(!presentationCmake.isEmpty());

    QVERIFY(source.contains(QStringLiteral("import QtQuick.Dialogs")));
    QVERIFY(source.contains(QStringLiteral("FileDialog {")));
    QVERIFY(source.contains(QStringLiteral("fileMode: FileDialog.OpenFile")));
    QVERIFY(source.contains(QStringLiteral("signal localFileSelected(url sourceUrl)")));
    QVERIFY(source.contains(QStringLiteral("onAccepted: root.localFileSelected(root.selectedFile)")));
    QVERIFY(presentationCmake.contains(QStringLiteral("QtQuick.Dialogs")));
    QVERIFY(!source.contains(QStringLiteral("onRejected")));
    QVERIFY(!source.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!source.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
    QVERIFY(!source.contains(QStringLiteral("mpv_"), Qt::CaseInsensitive));
}

void PlayerMediaOpenTest::mainWindowRoutesIntentThroughCoordinator()
{
    const QString source = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    QVERIFY(!source.isEmpty());

    QVERIFY(source.contains(QStringLiteral("property var mediaOpenCoordinator: null")));
    QVERIFY(source.contains(QStringLiteral("property var mediaViewModel: null")));
    QVERIFY(source.contains(QStringLiteral("LocalMediaOpenDialog {")));
    QVERIFY(source.contains(QStringLiteral("window.mediaOpenCoordinator.openLocalFile(sourceUrl)")));
    QVERIFY(source.contains(QStringLiteral("mediaViewModel: window.mediaViewModel")));
    QVERIFY(source.contains(QStringLiteral("modalActive: localMediaOpenDialog.visible")));
    QVERIFY(source.contains(QStringLiteral("onOpenMediaRequested: localMediaOpenDialog.open()")));
    QVERIFY(!source.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!source.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
    QVERIFY(!source.contains(QStringLiteral("mpv_"), Qt::CaseInsensitive));
}

void PlayerMediaOpenTest::playbackScreenAndOverlayRemainPresentationOnly()
{
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString overlay = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/overlays/status/PlayerStatusOverlay.qml"));
    QVERIFY(!screen.isEmpty());
    QVERIFY(!overlay.isEmpty());

    QVERIFY(screen.contains(QStringLiteral("signal openMediaRequested()")));
    QVERIFY(screen.contains(QStringLiteral("onOpenMediaRequested: root.openMediaRequested()")));
    QVERIFY(!screen.contains(QStringLiteral("mediaOpenCoordinator")));
    QVERIFY(!screen.contains(QStringLiteral("FileDialog")));

    QVERIFY(overlay.contains(QStringLiteral("EmptyFeedback")));
    QVERIFY(overlay.contains(QStringLiteral("signal openMediaRequested()")));
    QVERIFY(overlay.contains(QStringLiteral("onActionRequested: root.openMediaRequested()")));
    QVERIFY(!overlay.contains(QStringLiteral("mediaOpenCoordinator")));
    QVERIFY(!overlay.contains(QStringLiteral("FileDialog")));
}

void PlayerMediaOpenTest::bootstrapInjectsSingleCoordinatorBoundary()
{
    const QString bootstrap = readSource(QStringLiteral(
        "src/app/bootstrap/application_bootstrap.cpp"));
    const QString container = readSource(QStringLiteral(
        "src/app/composition/application_container.cpp"));
    const QString composition = readSource(QStringLiteral(
        "src/app/composition/playback_composition.cpp"));
    QVERIFY(!bootstrap.isEmpty());
    QVERIFY(!container.isEmpty());
    QVERIFY(!composition.isEmpty());

    QCOMPARE(bootstrap.count(QStringLiteral("mediaOpenCoordinator")), 2);
    QVERIFY(container.contains(QStringLiteral("MediaOpenCoordinator")));
    QVERIFY(container.contains(QStringLiteral("submitMediaLoad(source.location())")));
    QVERIFY(composition.contains(QStringLiteral("LoadMediaCommand{canonicalSource}")));
    QVERIFY(composition.contains(QStringLiteral("playbackThread_->commandBus()")));
}

void PlayerMediaOpenTest::mediaCapabilityUsesSnapshotProjection()
{
    const QString bootstrap = readSource(QStringLiteral(
        "src/app/bootstrap/application_bootstrap.cpp"));
    const QString composition = readSource(QStringLiteral(
        "src/app/composition/playback_composition.cpp"));
    const QString mediaViewModel = readSource(QStringLiteral(
        "src/presentation/viewmodels/player/media/player_media_view_model.cpp"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    QVERIFY(!bootstrap.isEmpty());
    QVERIFY(!composition.isEmpty());
    QVERIFY(!mediaViewModel.isEmpty());
    QVERIFY(!screen.isEmpty());

    QVERIFY(bootstrap.contains(QStringLiteral("playbackComposition.mediaViewModel()")));
    QVERIFY(composition.contains(QStringLiteral("PlayerMediaViewModel::acceptSnapshot")));
    QVERIFY(mediaViewModel.contains(QStringLiteral("snapshot.streams().video.has_value()")));
    QVERIFY(screen.contains(QStringLiteral("hasMedia: root.mediaViewModel !== null")));
    QVERIFY(screen.contains(QStringLiteral("hasVideo: root.mediaViewModel !== null")));
    QVERIFY(!mediaViewModel.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!mediaViewModel.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
}

void PlayerMediaOpenTest::localValidationAndCoordinationStaySeparated()
{
    const QString coordinator = readSource(QStringLiteral(
        "src/media/application/open/media_open_coordinator.cpp"));
    const QString validator = readSource(QStringLiteral(
        "src/media/application/open/local_media_validator.cpp"));
    QVERIFY(!coordinator.isEmpty());
    QVERIFY(!validator.isEmpty());

    QVERIFY(coordinator.contains(QStringLiteral("LocalMediaValidator::validate")));
    QVERIFY(!coordinator.contains(QStringLiteral("QFileInfo")));
    QVERIFY(validator.contains(QStringLiteral("QFileInfo")));
    QVERIFY(validator.contains(QStringLiteral("canonicalFilePath")));

    for (const QString* source : {&coordinator, &validator}) {
        QVERIFY(!source->contains(QStringLiteral("PlaybackSession")));
        QVERIFY(!source->contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
        QVERIFY(!source->contains(QStringLiteral("mpv_"), Qt::CaseInsensitive));
    }
}

} // namespace player::presentation::qml

QTEST_GUILESS_MAIN(player::presentation::qml::PlayerMediaOpenTest)
#include "player_media_open_test.moc"
