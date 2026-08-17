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
    void utilityControlsMatchCanonicalFigmaOrderAndSpacing();
    void inspectorShellMatchesCanonicalFigmaGeometry();
    void playlistContentFiltersReordersAndVirtualizesReadonlyRows();
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

    QVERIFY(!window.isEmpty());
    QVERIFY(!screen.isEmpty());

    QVERIFY(window.contains(QStringLiteral("playlistController: window.playlistController")));
    QVERIFY(window.contains(QStringLiteral("playlistModel: window.playlistModel")));
    QVERIFY(screen.contains(QStringLiteral("property var playlistController: null")));
    QVERIFY(screen.contains(QStringLiteral("property var playlistModel: null")));
    QVERIFY(screen.contains(QStringLiteral("property bool playlistDrawerOpen: false")));
    QVERIFY(screen.contains(QStringLiteral("PlaylistInspector {")));
    QVERIFY(screen.contains(QStringLiteral("playlistModel: root.playlistModel")));
    QVERIFY(screen.contains(QStringLiteral("playlistController: root.playlistController")));
    QVERIFY(screen.contains(QStringLiteral("inspectorOpen: root.playlistDrawerOpen && !root.fullScreen")));
}

void PlaylistQmlTest::utilityControlsMatchCanonicalFigmaOrderAndSpacing()
{
    const QString utility = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerUtilityControls.qml"));

    QVERIFY(!utility.isEmpty());
    QVERIFY(utility.contains(QStringLiteral("SpacingTokens.utilityControlGap")));
    QVERIFY(utility.contains(QStringLiteral("objectName: \"subtitlesUnavailableControl\"")));
    QVERIFY(utility.contains(QStringLiteral("iconId: \"subtitles\"")));
    QVERIFY(utility.contains(QStringLiteral("PlaylistControls {")));
    QVERIFY(utility.contains(QStringLiteral("FullscreenControls {")));
    QVERIFY(utility.indexOf(QStringLiteral("iconId: \"subtitles\""))
            < utility.indexOf(QStringLiteral("PlaylistControls {")));
    QVERIFY(utility.indexOf(QStringLiteral("PlaylistControls {"))
            < utility.indexOf(QStringLiteral("FullscreenControls {")));
}

void PlaylistQmlTest::inspectorShellMatchesCanonicalFigmaGeometry()
{
    const QString shell = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/PlayerInspectorShell.qml"));
    const QString inspector = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/PlaylistInspector.qml"));
    const QString search = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistSearchField.qml"));
    const QString footer = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistFooter.qml"));

    QVERIFY(!shell.isEmpty());
    QVERIFY(!inspector.isEmpty());
    QVERIFY(!search.isEmpty());
    QVERIFY(!footer.isEmpty());

    QVERIFY(shell.contains(QStringLiteral("Drawer {")));
    QVERIFY(shell.contains(QStringLiteral("property alias searchContent: searchHost.data")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.inspectorTitleTop")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.inspectorMetaTop")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.inspectorSearchTop")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.listSearchToFirst")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.inspectorFooterBottom")));
    QVERIFY(shell.contains(QStringLiteral("LayoutTokens.inspectorSearchHeight")));
    QVERIFY(shell.contains(QStringLiteral("LayoutTokens.inspectorFooterHeight")));
    QVERIFY(!shell.contains(QStringLiteral("iconId: \"close\"")));

    QVERIFY(inspector.contains(QStringLiteral("qsTr(\"播放列表\")")));
    QVERIFY(inspector.contains(QStringLiteral("PlaylistSearchField {")));
    QVERIFY(inspector.contains(QStringLiteral("filterText: searchField.text")));
    QVERIFY(inspector.contains(QStringLiteral("currentPosition")));

    QVERIFY(search.contains(QStringLiteral("LayoutTokens.inspectorSearchWidth")));
    QVERIFY(search.contains(QStringLiteral("LayoutTokens.inspectorSearchHeight")));
    QVERIFY(search.contains(QStringLiteral("RadiusTokens.controlSearch")));
    QVERIFY(search.contains(QStringLiteral("MaterialTokens.fieldBlur")));
    QVERIFY(search.contains(QStringLiteral("iconId: \"search\"")));
    QVERIFY(search.contains(QStringLiteral("qsTr(\"搜索播放列表\")")));

    QVERIFY(footer.contains(QStringLiteral("qsTr(\"拖放排序 · 双击播放\")")));
    QVERIFY(footer.contains(QStringLiteral("RadiusTokens.inspectorFooter")));
    QVERIFY(footer.contains(QStringLiteral("MaterialTokens.footerBlur")));
}

void PlaylistQmlTest::playlistContentFiltersReordersAndVirtualizesReadonlyRows()
{
    const QString content = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistContent.qml"));

    QVERIFY(!content.isEmpty());
    QVERIFY(content.contains(QStringLiteral("property string filterText: \"\"")));
    QVERIFY(content.contains(QStringLiteral("ListView {")));
    QVERIFY(content.contains(QStringLiteral("reuseItems: true")));
    QVERIFY(content.contains(QStringLiteral("model: root.playlistModel")));
    QVERIFY(content.contains(QStringLiteral("function matchesFilter")));
    QVERIFY(content.contains(QStringLiteral("matchesCurrentFilter")));
    QVERIFY(content.contains(QStringLiteral("DragHandler {")));
    QVERIFY(content.contains(QStringLiteral("root.playlistController.moveEntry(")));
    QVERIFY(content.contains(QStringLiteral("root.normalizedFilter.length === 0")));
    QVERIFY(content.contains(QStringLiteral("height: LayoutTokens.listRowHeight")));
    QVERIFY(content.contains(QStringLiteral("visible: root.itemCount === 0")));
}

void PlaylistQmlTest::playlistRowSeparatesCurrentSelectedHoverAndFocus()
{
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistRow.qml"));

    QVERIFY(!row.isEmpty());
    QVERIFY(row.contains(QStringLiteral("property bool selected: false")));
    QVERIFY(row.contains(QStringLiteral("required property bool current")));
    QVERIFY(row.contains(QStringLiteral("readonly property bool highlighted: root.current || root.selected")));
    QVERIFY(row.contains(QStringLiteral("MaterialTokens.selectionFillAlpha")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.borderSelection")));
    QVERIFY(row.contains(QStringLiteral("objectName: \"playlistPlayingRail\"")));
    QVERIFY(row.contains(QStringLiteral("LayoutTokens.listPlayingRailWidth")));
    QVERIFY(row.contains(QStringLiteral("visible: root.current")));
    QVERIFY(row.contains(QStringLiteral("TypographyTokens.mediaTitleCompact")));
    QVERIFY(row.contains(QStringLiteral("TypographyTokens.timecodeExtraSmall")));
    QVERIFY(row.contains(QStringLiteral("activeFocusOnTab: true")));
    QVERIFY(row.contains(QStringLiteral("onDoubleTapped: root.activationRequested(root.entryId)")));
}

void PlaylistQmlTest::playlistActionsFlowThroughControllerOnly()
{
    const QString content = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistContent.qml"));
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistRow.qml"));
    const QString controls = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistControls.qml"));

    QVERIFY(content.contains(QStringLiteral("root.playlistController.selectEntry(requestedEntryId)")));
    QVERIFY(content.contains(QStringLiteral("root.playlistController.removeEntry(requestedEntryId)")));
    QVERIFY(content.contains(QStringLiteral("root.playlistController.moveEntry(")));
    QVERIFY(controls.contains(QStringLiteral("signal toggleRequested()")));

    const QStringList sources{content, row};
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
        QStringLiteral("qml/features/playlist/PlaylistSearchField.qml"),
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
