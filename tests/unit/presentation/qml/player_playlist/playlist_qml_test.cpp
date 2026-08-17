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
    void darkInspectorUsesCanonicalFigmaPalette();
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
    QVERIFY(screen.contains(QStringLiteral("backdropSource: videoViewport")));
    QVERIFY(screen.contains(QStringLiteral("backdropMappingRevision: drawerHost.x + drawerHost.y")));
}

void PlaylistQmlTest::utilityControlsMatchCanonicalFigmaOrderAndSpacing()
{
    const QString utility = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerUtilityControls.qml"));

    QVERIFY(!utility.isEmpty());
    QVERIFY(utility.contains(QStringLiteral("SpacingTokens.utilityControlGap")));
    QVERIFY(utility.contains(QStringLiteral("SpacingTokens.fullscreenTransportGap")));
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
        "src/presentation/qml/screens/player/drawers/PlaylistSearchField.qml"));
    const QString footer = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/PlaylistFooter.qml"));
    const QString panel = readSource(QStringLiteral(
        "src/presentation/qml/surfaces/Panel.qml"));
    const QString drawer = readSource(QStringLiteral(
        "src/presentation/qml/surfaces/Drawer.qml"));
    const QString backdropBlur = readSource(QStringLiteral(
        "src/presentation/qml/surfaces/BackdropBlur.qml"));
    const QString surfaceCMake = readSource(QStringLiteral(
        "src/presentation/qml/surfaces/CMakeLists.txt"));

    QVERIFY(!shell.isEmpty());
    QVERIFY(!inspector.isEmpty());
    QVERIFY(!search.isEmpty());
    QVERIFY(!footer.isEmpty());
    QVERIFY(!panel.isEmpty());
    QVERIFY(!drawer.isEmpty());
    QVERIFY(!backdropBlur.isEmpty());
    QVERIFY(!surfaceCMake.isEmpty());

    QVERIFY(shell.contains(QStringLiteral("Drawer {")));
    QVERIFY(shell.contains(QStringLiteral("property Item backdropSource: null")));
    QVERIFY(shell.contains(QStringLiteral("property real backdropMappingRevision: 0")));
    QVERIFY(shell.contains(QStringLiteral("backdropSource: root.backdropSource")));
    QVERIFY(shell.contains(QStringLiteral(
        "backdropMappingRevision: root.backdropMappingRevision + shellTranslation.x")));
    QVERIFY(shell.contains(QStringLiteral("property alias searchContent: searchHost.data")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.inspectorTitleTop")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.inspectorMetaTop")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.inspectorSearchTop")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.listSearchToFirst")));
    QVERIFY(shell.contains(QStringLiteral("SpacingTokens.inspectorFooterBottom")));
    QVERIFY(shell.contains(QStringLiteral("LayoutTokens.inspectorSearchHeight")));
    QVERIFY(shell.contains(QStringLiteral("LayoutTokens.inspectorFooterHeight")));
    QVERIFY(!shell.contains(QStringLiteral("iconId: \"close\"")));

    QVERIFY(inspector.contains(QStringLiteral("property Item backdropSource: null")));
    QVERIFY(inspector.contains(QStringLiteral("backdropSource: root.backdropSource")));
    QVERIFY(inspector.contains(QStringLiteral("qsTr(\"播放列表\")")));
    QVERIFY(inspector.contains(QStringLiteral("PlaylistSearchField {")));
    QVERIFY(inspector.contains(QStringLiteral("filterText: searchField.text")));
    QVERIFY(inspector.contains(QStringLiteral("currentPosition")));

    QVERIFY(panel.contains(QStringLiteral("property Item backdropSource: null")));
    QVERIFY(panel.contains(QStringLiteral("BackdropBlur {")));
    QVERIFY(panel.contains(QStringLiteral("sourceItem: root.backdropSource")));
    QVERIFY(panel.contains(QStringLiteral("blurRadius: root.backdropBlurRadius")));
    QVERIFY(panel.contains(QStringLiteral("property real shadowXOffset: 0.0")));
    QVERIFY(panel.contains(QStringLiteral("shadowHorizontalOffset: root.shadowXOffset")));
    QVERIFY(drawer.contains(QStringLiteral(
        "borderAlpha: MaterialTokens.borderStrongAlpha")));

    QVERIFY(backdropBlur.contains(QStringLiteral("ShaderEffectSource {")));
    QVERIFY(backdropBlur.contains(QStringLiteral("sourceRect: root.mappedSourceRect")));
    QVERIFY(backdropBlur.contains(QStringLiteral("blurEnabled: true")));
    QVERIFY(backdropBlur.contains(QStringLiteral("blurMax: root.blurRadius")));
    QVERIFY(backdropBlur.contains(QStringLiteral("maskSource: roundedMask")));

    QVERIFY(search.contains(QStringLiteral("LayoutTokens.inspectorSearchWidth")));
    QVERIFY(search.contains(QStringLiteral("LayoutTokens.inspectorSearchHeight")));
    QVERIFY(search.contains(QStringLiteral("RadiusTokens.controlSearch")));
    QVERIFY(search.contains(QStringLiteral("MaterialTokens.inspectorDarkFieldBlur")));
    QVERIFY(search.contains(QStringLiteral(
        "borderAlpha: MaterialTokens.inspectorDarkFieldBorderAlpha")));
    QVERIFY(search.contains(QStringLiteral("iconId: \"search\"")));
    QVERIFY(search.contains(QStringLiteral("qsTr(\"搜索播放列表\")")));

    QVERIFY(footer.contains(QStringLiteral("qsTr(\"拖放排序 · 双击播放\")")));
    QVERIFY(footer.contains(QStringLiteral("RadiusTokens.inspectorFooter")));
    QVERIFY(footer.contains(QStringLiteral("MaterialTokens.inspectorDarkFooterBlur")));
    QVERIFY(footer.contains(QStringLiteral(
        "borderAlpha: MaterialTokens.inspectorDarkFooterBorderAlpha")));

    QVERIFY(surfaceCMake.contains(QStringLiteral("BackdropBlur.qml")));
    QVERIFY(surfaceCMake.contains(QStringLiteral("QT_QML_INTERNAL_TYPE TRUE")));
}

void PlaylistQmlTest::darkInspectorUsesCanonicalFigmaPalette()
{
    const QString colors = readSource(QStringLiteral(
        "src/presentation/qml/theme/ColorPrimitives.qml"));
    const QString colorTokens = readSource(QStringLiteral(
        "src/presentation/qml/theme/ColorTokens.qml"));
    const QString materials = readSource(QStringLiteral(
        "src/presentation/qml/theme/MaterialTokens.qml"));
    const QString elevation = readSource(QStringLiteral(
        "src/presentation/qml/theme/ElevationTokens.qml"));
    const QString shell = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/PlayerInspectorShell.qml"));

    QVERIFY(!colors.isEmpty());
    QVERIFY(!colorTokens.isEmpty());
    QVERIFY(!materials.isEmpty());
    QVERIFY(!elevation.isEmpty());
    QVERIFY(!shell.isEmpty());

    QVERIFY(colors.contains(QStringLiteral("neutralNight950: \"#17141F\"")));
    QVERIFY(colors.contains(QStringLiteral("neutralNight850: \"#27212F\"")));
    QVERIFY(colors.contains(QStringLiteral("neutralNight900: \"#211B29\"")));
    QVERIFY(colors.contains(QStringLiteral("violetNightSelected: \"#3A2852\"")));
    QVERIFY(colors.contains(QStringLiteral("violet300: \"#A879FF\"")));
    QVERIFY(colors.contains(QStringLiteral("violet200: \"#C49AFF\"")));

    QVERIFY(colorTokens.contains(QStringLiteral(
        "surfaceInspectorDark: ColorPrimitives.neutralNight950")));
    QVERIFY(colorTokens.contains(QStringLiteral(
        "textInspectorPrimary: ColorPrimitives.neutralLavender50")));
    QVERIFY(colorTokens.contains(QStringLiteral(
        "borderInspectorSelection: ColorPrimitives.violet300")));

    QVERIFY(materials.contains(QStringLiteral(
        "inspectorDarkFillAlpha: MaterialAlphaPrimitives.a94")));
    QVERIFY(materials.contains(QStringLiteral(
        "inspectorDarkFieldFillAlpha: MaterialAlphaPrimitives.a96")));
    QVERIFY(materials.contains(QStringLiteral(
        "inspectorDarkRowFillAlpha: MaterialAlphaPrimitives.a68")));
    QVERIFY(materials.contains(QStringLiteral(
        "inspectorDarkSelectionFillAlpha: MaterialAlphaPrimitives.a98")));
    QVERIFY(materials.contains(QStringLiteral(
        "inspectorDarkSelectionBorderAlpha: MaterialAlphaPrimitives.a72")));

    QVERIFY(elevation.contains(QStringLiteral(
        "inspectorDarkShadowXOffset: ShadowPrimitives.xMinus6")));
    QVERIFY(elevation.contains(QStringLiteral(
        "inspectorDarkShadowYOffset: ShadowPrimitives.y18")));
    QVERIFY(elevation.contains(QStringLiteral(
        "inspectorDarkShadowAlpha: MaterialAlphaPrimitives.a48")));

    QVERIFY(shell.contains(QStringLiteral("surfaceColor: ColorTokens.surfaceInspectorDark")));
    QVERIFY(shell.contains(QStringLiteral("fillAlpha: MaterialTokens.inspectorDarkFillAlpha")));
    QVERIFY(shell.contains(QStringLiteral("borderAlpha: MaterialTokens.inspectorDarkBorderAlpha")));
    QVERIFY(shell.contains(QStringLiteral("backdropBlurRadius: MaterialTokens.inspectorDarkBlur")));
    QVERIFY(shell.contains(QStringLiteral("shadowXOffset: ElevationTokens.inspectorDarkShadowXOffset")));
    QVERIFY(shell.contains(QStringLiteral("color: ColorTokens.textInspectorPrimary")));
    QVERIFY(shell.contains(QStringLiteral("color: ColorTokens.textInspectorSecondary")));
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
    QVERIFY(content.contains(QStringLiteral("ColorTokens.textInspectorSecondary")));
    QVERIFY(content.contains(QStringLiteral("ColorTokens.textInspectorMuted")));
}

void PlaylistQmlTest::playlistRowSeparatesCurrentSelectedHoverAndFocus()
{
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistRow.qml"));

    QVERIFY(!row.isEmpty());
    QVERIFY(row.contains(QStringLiteral("property bool selected: false")));
    QVERIFY(row.contains(QStringLiteral("required property bool current")));
    QVERIFY(row.contains(QStringLiteral("readonly property bool highlighted: root.current || root.selected")));
    QVERIFY(row.contains(QStringLiteral("MaterialTokens.inspectorDarkSelectionFillAlpha")));
    QVERIFY(row.contains(QStringLiteral("MaterialTokens.inspectorDarkRowFillAlpha")));
    QVERIFY(row.contains(QStringLiteral("MaterialTokens.inspectorDarkRowHoverAlpha")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.borderInspectorSelection")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.textInspectorStrong")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.textInspectorMuted")));
    QVERIFY(row.contains(QStringLiteral("objectName: \"playlistPlayingRail\"")));
    QVERIFY(row.contains(QStringLiteral("LayoutTokens.listPlayingRailWidth")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.selectionIndicatorInspector")));
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
        QStringLiteral("qml/screens/player/drawers/PlaylistSearchField.qml"),
        QStringLiteral("qml/screens/player/drawers/PlaylistFooter.qml"),
        QStringLiteral("qml/screens/player/osc/PlayerUtilityControls.qml"),
        QStringLiteral("qml/features/playlist/PlaylistControls.qml"),
        QStringLiteral("qml/features/playlist/PlaylistContent.qml"),
        QStringLiteral("qml/features/playlist/PlaylistRow.qml"),
    };

    for (const QString& path : requiredPaths) {
        QVERIFY2(cmake.contains(path), qPrintable(path));
    }
}

} // namespace player::presentation::qml

QTEST_MAIN(player::presentation::qml::PlaylistQmlTest)
#include "playlist_qml_test.moc"
