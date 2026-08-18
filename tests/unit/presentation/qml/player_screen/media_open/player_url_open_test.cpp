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

class PlayerUrlOpenTest final : public QObject
{
    Q_OBJECT

private slots:
    void urlDialogUsesPublicPresentationBoundaryOnly();
    void mainWindowRoutesUrlIntentThroughWorkflow();
    void emptyStateExposesSeparateUrlIntent();
    void remoteOpenRecoveryIsExplicit();
    void urlWorkflowUsesUnifiedCoordinatorBoundary();
    void validatorOwnsHttpHttpsStructureRules();
    void bootstrapInjectsUrlWorkflow();
};

void PlayerUrlOpenTest::urlDialogUsesPublicPresentationBoundaryOnly()
{
    const QString dialog = readSource(QStringLiteral(
        "src/presentation/qml/features/player/mediaopen/UrlMediaOpenDialog.qml"));
    QVERIFY(!dialog.isEmpty());

    QVERIFY(dialog.contains(QStringLiteral("import QtQuick.Controls.Basic")));
    QVERIFY(!dialog.contains(QStringLiteral("import QtQuick.Controls\n")));
    QVERIFY(dialog.contains(QStringLiteral("Popup {")));
    QVERIFY(dialog.contains(QStringLiteral("TextField {")));
    QVERIFY(dialog.contains(QStringLiteral("Rectangle {")));
    QVERIFY(dialog.contains(QStringLiteral("TextButton {")));
    QVERIFY(dialog.contains(QStringLiteral("signal urlSubmitted(string sourceText)")));
    QVERIFY(dialog.contains(QStringLiteral("Only HTTP and HTTPS URLs are supported")));
    QVERIFY(dialog.contains(QStringLiteral("RadiusTokens.surfaceDialog")));
    QVERIFY(dialog.contains(QStringLiteral("MaterialTokens.popoverFillAlpha")));
    QVERIFY(dialog.contains(QStringLiteral("import Player.Presentation.Theme")));
    QVERIFY(dialog.contains(QStringLiteral("import Player.Presentation.Controls")));
    QVERIFY(!dialog.contains(QStringLiteral("Player.Presentation.Primitives")));
    QVERIFY(!dialog.contains(QStringLiteral("Player.Presentation.Surfaces")));
    QVERIFY(!dialog.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!dialog.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
    QVERIFY(!dialog.contains(QStringLiteral("mpv_"), Qt::CaseInsensitive));
}

void PlayerUrlOpenTest::mainWindowRoutesUrlIntentThroughWorkflow()
{
    const QString mainWindow = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    QVERIFY(!mainWindow.isEmpty());

    QVERIFY(mainWindow.contains(QStringLiteral("property var urlOpenWorkflow: null")));
    QVERIFY(mainWindow.contains(QStringLiteral("UrlMediaOpenDialog {")));
    QVERIFY(mainWindow.contains(QStringLiteral("window.urlOpenWorkflow.openUrl(sourceText)")));
    QVERIFY(mainWindow.contains(QStringLiteral("urlMediaOpenDialog.errorKey = window.urlOpenWorkflow.lastErrorKey")));
    QVERIFY(mainWindow.contains(QStringLiteral(
        "modalActive: localMediaOpenDialog.visible || urlMediaOpenDialog.visible")));
    QVERIFY(mainWindow.contains(QStringLiteral("onOpenUrlRequested: urlMediaOpenDialog.open()")));
    QVERIFY(!mainWindow.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!mainWindow.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
    QVERIFY(!mainWindow.contains(QStringLiteral("mpv_"), Qt::CaseInsensitive));
}

void PlayerUrlOpenTest::emptyStateExposesSeparateUrlIntent()
{
    const QString overlay = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/overlays/status/PlayerStatusOverlay.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    QVERIFY(!overlay.isEmpty());
    QVERIFY(!screen.isEmpty());

    QVERIFY(overlay.contains(QStringLiteral("signal openUrlRequested()")));
    QVERIFY(overlay.contains(QStringLiteral("objectName: \"playerOpenUrlAction\"")));
    QVERIFY(overlay.contains(QStringLiteral("text: qsTr(\"Open URL\")")));
    QVERIFY(overlay.contains(QStringLiteral("onClicked: root.openUrlRequested()")));
    QVERIFY(screen.contains(QStringLiteral("signal openUrlRequested()")));
    QVERIFY(screen.contains(QStringLiteral("onOpenUrlRequested: root.openUrlRequested()")));
    QVERIFY(!overlay.contains(QStringLiteral("urlOpenWorkflow")));
    QVERIFY(!screen.contains(QStringLiteral("urlOpenWorkflow")));
}

void PlayerUrlOpenTest::remoteOpenRecoveryIsExplicit()
{
    const QString overlay = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/overlays/status/PlayerStatusOverlay.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString tracker = readSource(QStringLiteral(
        "src/playback/application/requests/request_tracker.cpp"));
    const QString monitor = readSource(QStringLiteral(
        "src/playback/application/requests/request_timeout_monitor.cpp"));
    const QString session = readSource(QStringLiteral(
        "src/playback/application/session/playback_session.cpp"));
    QVERIFY(!overlay.isEmpty());
    QVERIFY(!screen.isEmpty());
    QVERIFY(!tracker.isEmpty());
    QVERIFY(!monitor.isEmpty());
    QVERIFY(!session.isEmpty());

    QVERIFY(overlay.contains(QStringLiteral("signal cancelMediaOpenRequested()")));
    QVERIFY(overlay.contains(QStringLiteral("objectName: \"playerCancelMediaOpenAction\"")));
    QVERIFY(overlay.contains(QStringLiteral("text: qsTr(\"Cancel\")")));
    QVERIFY(overlay.contains(QStringLiteral("onClicked: root.cancelMediaOpenRequested()")));
    QVERIFY(overlay.contains(QStringLiteral("actionText: qsTr(\"Open media\")")));
    QVERIFY(screen.contains(QStringLiteral("onCancelMediaOpenRequested:")));
    QVERIFY(screen.contains(QStringLiteral("root.transportViewModel.requestStop()")));

    QVERIFY(tracker.contains(QStringLiteral("RequestTracker::cancelExpiredRecords")));
    QVERIFY(monitor.contains(QStringLiteral("tracker_.cancelExpiredRecords")));
    QVERIFY(monitor.contains(QStringLiteral("timeoutHandler_(record)")));
    QVERIFY(session.contains(QStringLiteral("handleRequestTimeout(record)")));
    QVERIFY(session.contains(QStringLiteral("record.type != PlaybackRequestType::LoadMedia")));
    QVERIFY(session.contains(QStringLiteral("snapshot_.lifecycle() != PlaybackLifecycleState::Opening")));
    QVERIFY(session.contains(QStringLiteral("Media load timed out.")));
    QVERIFY(session.contains(QStringLiteral("mediaGenerationGate_.reset()")));
    QVERIFY(session.contains(QStringLiteral("MediaEndReason::Stopped")));
    QVERIFY(session.contains(QStringLiteral("MediaFailedEvent")));
}

void PlayerUrlOpenTest::urlWorkflowUsesUnifiedCoordinatorBoundary()
{
    const QString workflow = readSource(QStringLiteral(
        "src/media/application/open/url_open_workflow.cpp"));
    const QString coordinator = readSource(QStringLiteral(
        "src/media/application/open/media_open_coordinator.cpp"));
    QVERIFY(!workflow.isEmpty());
    QVERIFY(!coordinator.isEmpty());

    QVERIFY(workflow.contains(QStringLiteral("UrlMediaValidator::validate")));
    QVERIFY(workflow.contains(QStringLiteral(
        "mediaOpenCoordinator_.beginReplaceOpenOperation()")));
    QVERIFY(workflow.contains(QStringLiteral(
        "mediaOpenCoordinator_.completeOpenSource(operationId, source)")));
    QVERIFY(workflow.contains(QStringLiteral(
        "mediaOpenCoordinator_.cancelOpenOperation(operationId)")));
    QVERIFY(coordinator.contains(QStringLiteral(
        "MediaOpenCoordinator::beginReplaceOpenOperation")));
    QVERIFY(coordinator.contains(QStringLiteral(
        "MediaOpenCoordinator::completeOpenSource")));
    QVERIFY(coordinator.contains(QStringLiteral("submitMedia_(source)")));
    QVERIFY(!workflow.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!workflow.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
    QVERIFY(!workflow.contains(QStringLiteral("mpv_"), Qt::CaseInsensitive));
}

void PlayerUrlOpenTest::validatorOwnsHttpHttpsStructureRules()
{
    const QString validator = readSource(QStringLiteral(
        "src/media/application/open/url_media_validator.cpp"));
    QVERIFY(!validator.isEmpty());

    QVERIFY(validator.contains(QStringLiteral("QUrl::StrictMode")));
    QVERIFY(validator.contains(QStringLiteral("sourceUrl.host().isEmpty()")));
    QVERIFY(validator.contains(QStringLiteral("QStringLiteral(\"http\")")));
    QVERIFY(validator.contains(QStringLiteral("QStringLiteral(\"https\")")));
    QVERIFY(validator.contains(QStringLiteral("MediaSource::remoteUrl")));
    QVERIFY(!validator.contains(QStringLiteral("QNetworkAccessManager")));
    QVERIFY(!validator.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!validator.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
}

void PlayerUrlOpenTest::bootstrapInjectsUrlWorkflow()
{
    const QString bootstrap = readSource(QStringLiteral(
        "src/app/bootstrap/application_bootstrap.cpp"));
    const QString container = readSource(QStringLiteral(
        "src/app/composition/application_container.cpp"));
    QVERIFY(!bootstrap.isEmpty());
    QVERIFY(!container.isEmpty());

    QVERIFY(bootstrap.contains(QStringLiteral("QStringLiteral(\"urlOpenWorkflow\")")));
    QVERIFY(bootstrap.contains(QStringLiteral("container.urlOpenWorkflow()")));
    QVERIFY(container.contains(QStringLiteral("UrlOpenWorkflow")));
    QVERIFY(container.contains(QStringLiteral("*mediaOpenCoordinator_")));
}

} // namespace player::presentation::qml

QTEST_GUILESS_MAIN(player::presentation::qml::PlayerUrlOpenTest)
#include "player_url_open_test.moc"
