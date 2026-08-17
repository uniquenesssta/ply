#include <QFile>
#include <QString>
#include <QStringList>
#include <QtTest>

namespace player::presentation::qml {
namespace {

QString sourcePath(const QString& relativePath)
{
    return QStringLiteral(PLAYER_SOURCE_DIR) + QLatin1Char('/') + relativePath;
}

QString readSource(const QString& relativePath)
{
    QFile file(sourcePath(relativePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

void verifyAbsent(const QString& source, const QStringList& forbidden)
{
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

} // namespace

class PlaylistQmlTest final : public QObject
{
    Q_OBJECT

private slots:
    void screenRoutesPlaylistThroughReadonlyPresentationBoundary();
    void utilityControlsKeepConcreteGeometryWithoutOverridingImplicitSize();
    void inspectorShellOwnsSharedDrawerSurfaceAndMotion();
    void playlistContentSeparatesSelectionFromCurrentAndVirtualizes();
    void playlistRowSeparatesCurrentSelectedHoverAndFocus();
    void playlistActionsFlowThroughControllerOnly();
    void qmlRegistrationIncludesPlaylistModules();
};

void PlaylistQmlTest::screenRoutesPlaylistThroughReadonlyPresentationBoundary()
{
    const QString window = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString utility = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerUtilityControls.qml"));

    QVERIFY(!window.isEmpty());
    QVERIFY(!screen.isEmpty());
    QVERIFY(!utility.isEmpty());

    QVERIFY(window.contains(QStringLiteral("playlistController: window.playlistController")));
    QVERIFY(window.contains(QStringLiteral("playlistModel: window.playlistModel")));

    QVERIFY(screen.contains(QStringLiteral("property var playlistController: null")));
    QVERIFY(screen.contains(QStringLiteral("property var playlistModel: null")));
    QVERIFY(screen.contains(QStringLiteral("property bool playlistDrawerOpen: false")));
    QVERIFY(screen.contains(QStringLiteral(
        "readonly property bool anyDrawerOpen: root.drawerOpen || root.playlistDrawerOpen")));
    QVERIFY(screen.contains(QStringLiteral("drawerOpen: root.anyDrawerOpen")));
    QVERIFY(screen.contains(QStringLiteral("PlayerUtilityControls {")));
    QVERIFY(screen.contains(QStringLiteral("PlaylistInspector {")));
    QVERIFY(screen.contains(QStringLiteral("playlistModel: root.playlistModel")));
    QVERIFY(screen.contains(QStringLiteral("playlistController: root.playlistController")));
    QVERIFY(screen.contains(QStringLiteral("onAddMediaRequested: root.openMediaRequested()")));

    QVERIFY(utility.contains(QStringLiteral("PlaylistControls {")));
    QVERIFY(utility.contains(QStringLiteral("FullscreenControls {")));
    QVERIFY(utility.contains(QStringLiteral("SpacingTokens.controlTight")));
    QVERIFY(utility.indexOf(QStringLiteral("PlaylistControls {"))
            < utility.indexOf(QStringLiteral("FullscreenControls {")));
}

void PlaylistQmlTest::utilityControlsKeepConcreteGeometryWithoutOverridingImplicitSize()
{
    const QString utility = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerUtilityControls.qml"));
    const QString playlistControls = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistControls.qml"));
    const QString fullscreenControls = readSource(QStringLiteral(
        "src/presentation/qml/features/player/fullscreen/FullscreenControls.qml"));

    QVERIFY(!utility.isEmpty());
    QVERIFY(!playlistControls.isEmpty());
    QVERIFY(!fullscreenControls.isEmpty());

    QVERIFY(utility.contains(QStringLiteral("width: implicitWidth")));
    QVERIFY(utility.contains(QStringLiteral("height: implicitHeight")));
    QVERIFY(utility.contains(QStringLiteral("id: playlistControls")));
    QVERIFY(utility.contains(QStringLiteral("id: fullscreenControls")));

    QVERIFY(playlistControls.contains(QStringLiteral("width: implicitWidth")));
    QVERIFY(playlistControls.contains(QStringLiteral("height: implicitHeight")));
    QVERIFY(!playlistControls.contains(QStringLiteral(
        "implicitWidth: playlistButton.implicitWidth")));
    QVERIFY(!playlistControls.contains(QStringLiteral(
        "implicitHeight: playlistButton.implicitHeight")));

    QVERIFY(fullscreenControls.contains(QStringLiteral("width: implicitWidth")));
    QVERIFY(fullscreenControls.contains(QStringLiteral("height: implicitHeight")));
    QVERIFY(!fullscreenControls.contains(QStringLiteral(
        "implicitWidth: fullscreenButton.implicitWidth")));
    QVERIFY(!fullscreenControls.contains(QStringLiteral(
        "implicitHeight: fullscreenButton.implicitHeight")));
}

void PlaylistQmlTest::inspectorShellOwnsSharedDrawerSurfaceAndMotion()
{
    const QString shell = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/PlayerInspectorShell.qml"));
    const QString playlistInspector = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/PlaylistInspector.qml"));

    QVERIFY(!shell.isEmpty());
    QVERIFY(!playlistInspector.isEmpty());

    QVERIFY(shell.contains(QStringLiteral("import Player.Presentation.Surfaces")));
    QVERIFY(shell.contains(QStringLiteral("Drawer {")));
    QVERIFY(shell.contains(QStringLiteral("property alias bodyContent: bodyHost.data")));
    QVERIFY(shell.contains(QStringLiteral("property alias footerContent: footerHost.data")));
    QVERIFY(shell.contains(QStringLiteral("LayoutTokens.headerHeightCompact")));
    QVERIFY(shell.contains(QStringLiteral("LayoutTokens.inspectorFooterHeight")));
    QVERIFY(shell.contains(QStringLiteral("RadiusTokens.inspectorFooter")));
    QVERIFY(shell.contains(QStringLiteral("MotionTokens.inspectorOpenDuration")));
    QVERIFY(shell.contains(QStringLiteral("MotionTokens.inspectorCloseDuration")));
    QVERIFY(shell.contains(QStringLiteral("iconId: \"close\"")));

    QVERIFY(playlistInspector.contains(QStringLiteral("PlayerInspectorShell {")));
    QVERIFY(playlistInspector.contains(QStringLiteral("PlaylistContent {")));
    QVERIFY(playlistInspector.contains(QStringLiteral("PlaylistFooter {")));
    QVERIFY(!playlistInspector.contains(QStringLiteral("Drawer {")));
}

void PlaylistQmlTest::playlistContentSeparatesSelectionFromCurrentAndVirtualizes()
{
    const QString content = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistContent.qml"));
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistRow.qml"));

    QVERIFY(!content.isEmpty());
    QVERIFY(!row.isEmpty());

    QVERIFY(content.contains(QStringLiteral("property var selectedEntryId: null")));
    QVERIFY(content.contains(QStringLiteral("ListView {")));
    QVERIFY(content.contains(QStringLiteral("reuseItems: true")));
    QVERIFY(content.contains(QStringLiteral("model: root.playlistModel")));
    QVERIFY(content.contains(QStringLiteral("selected: root.selectedEntryId")));
    QVERIFY(content.contains(QStringLiteral("required property bool current")));
    QVERIFY(content.contains(QStringLiteral("visible: root.itemCount === 0")));
    QVERIFY(content.contains(QStringLiteral("qsTr(\"Playlist is empty\")")));

    QVERIFY(row.contains(QStringLiteral("property bool selected: false")));
    QVERIFY(row.contains(QStringLiteral("required property bool current")));
    QVERIFY(row.contains(QStringLiteral("readonly property bool hovered: rowHover.hovered")));
    QVERIFY(row.contains(QStringLiteral("activeFocusOnTab: true")));
    QVERIFY(row.contains(QStringLiteral("Text.ElideRight")));
    QVERIFY(row.contains(QStringLiteral("Text.ElideMiddle")));
}

void PlaylistQmlTest::playlistRowSeparatesCurrentSelectedHoverAndFocus()
{
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistRow.qml"));

    QVERIFY(!row.isEmpty());

    QVERIFY(row.contains(QStringLiteral("MaterialTokens.selectionFillAlpha")));
    QVERIFY(row.contains(QStringLiteral("MaterialTokens.rowFillAlpha")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.accentStrong")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.focusRing")));
    QVERIFY(row.contains(QStringLiteral("RadiusTokens.listRow")));
    QVERIFY(row.contains(QStringLiteral("LayoutTokens.listRowHeight")));
    QVERIFY(row.contains(QStringLiteral("enabled: !root.current")));
    QVERIFY(row.contains(QStringLiteral("TapHandler {")));
    QVERIFY(row.contains(QStringLiteral("onDoubleTapped: root.activationRequested(root.entryId)")));
    QVERIFY(row.contains(QStringLiteral("Qt.Key_Return")));
    QVERIFY(row.contains(QStringLiteral("Qt.Key_Space")));
}

void PlaylistQmlTest::playlistActionsFlowThroughControllerOnly()
{
    const QString content = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistContent.qml"));
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistRow.qml"));
    const QString footer = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistFooter.qml"));
    const QString controls = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistControls.qml"));

    QVERIFY(content.contains(QStringLiteral("root.playlistController.selectEntry(requestedEntryId)")));
    QVERIFY(content.contains(QStringLiteral("root.playlistController.removeEntry(requestedEntryId)")));
    QVERIFY(footer.contains(QStringLiteral("signal addMediaRequested()")));
    QVERIFY(controls.contains(QStringLiteral("signal toggleRequested()")));

    const QStringList sources{content, row, footer, controls};
    for (const QString& source : sources) {
        verifyAbsent(source,
                     {QStringLiteral("PlaybackSession"),
                      QStringLiteral("PlaybackCommandBus"),
                      QStringLiteral("libmpv"),
                      QStringLiteral("mpv_"),
                      QStringLiteral("append("),
                      QStringLiteral("setData("),
                      QStringLiteral("removeRow(")});
    }
}

void PlaylistQmlTest::qmlRegistrationIncludesPlaylistModules()
{
    const QString cmake = readSource(QStringLiteral(
        "src/presentation/CMakeLists.txt"));

    QVERIFY(!cmake.isEmpty());

    const QStringList requiredPaths{
        QStringLiteral("qml/screens/player/drawers/PlayerInspectorShell.qml"),
        QStringLiteral("qml/screens/player/drawers/PlaylistInspector.qml"),
        QStringLiteral("qml/screens/player/osc/PlayerUtilityControls.qml"),
        QStringLiteral("qml/features/playlist/PlaylistControls.qml"),
        QStringLiteral("qml/features/playlist/PlaylistContent.qml"),
        QStringLiteral("qml/features/playlist/PlaylistRow.qml"),
        QStringLiteral("qml/features/playlist/PlaylistFooter.qml"),
    };

    for (const QString& path : requiredPaths) {
        QVERIFY2(cmake.contains(path), qPrintable(path));
    }
}

} // namespace player::presentation::qml

QTEST_MAIN(player::presentation::qml::PlaylistQmlTest)
#include "playlist_qml_test.moc"
