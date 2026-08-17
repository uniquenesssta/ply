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

class FullscreenControlsTest final : public QObject
{
    Q_OBJECT

private slots:
    void windowControllerOwnsQtFullscreenStateAndRestore();
    void screenRoutesButtonDoubleClickAndWindowState();
    void fullscreenPresentationMatchesCanonicalFigmaGeometry();
    void sharedControlsAdaptWithoutDuplicatingPlaybackState();
    void platformSpecificWindowWorkRemainsDeferred();
};

void FullscreenControlsTest::windowControllerOwnsQtFullscreenStateAndRestore()
{
    const QString controller = readSource(QStringLiteral(
        "src/presentation/qml/shell/window/FullscreenWindowController.qml"));
    const QString window = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));

    QVERIFY(!controller.isEmpty());
    QVERIFY(!window.isEmpty());

    QVERIFY(controller.contains(QStringLiteral("property var targetWindow: null")));
    QVERIFY(controller.contains(QStringLiteral("readonly property bool fullScreen")));
    QVERIFY(controller.contains(QStringLiteral("Window.FullScreen")));
    QVERIFY(controller.contains(QStringLiteral("Window.Maximized")));
    QVERIFY(controller.contains(QStringLiteral("showFullScreen()")));
    QVERIFY(controller.contains(QStringLiteral("showMaximized()")));
    QVERIFY(controller.contains(QStringLiteral("showNormal()")));
    QVERIFY(controller.contains(QStringLiteral("sequence: \"Esc\"")));
    QVERIFY(controller.contains(QStringLiteral("Qt.ApplicationShortcut")));
    QVERIFY(controller.contains(QStringLiteral("onActivated: root.exitFullscreen()")));

    QVERIFY(window.contains(QStringLiteral("FullscreenWindowController {")));
    QVERIFY(window.contains(QStringLiteral("targetWindow: window")));
    QVERIFY(window.contains(QStringLiteral(
        "fullScreen: fullscreenWindowController.fullScreen")));
    QVERIFY(window.contains(QStringLiteral(
        "onFullscreenToggleRequested: fullscreenWindowController.toggleFullscreen()")));
    QVERIFY(window.contains(QStringLiteral("fullscreenWindowController.exitFullscreen()")));
}

void FullscreenControlsTest::screenRoutesButtonDoubleClickAndWindowState()
{
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString utility = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerUtilityControls.qml"));
    const QString gesture = readSource(QStringLiteral(
        "src/presentation/qml/features/player/fullscreen/FullscreenGestureLayer.qml"));
    const QString controls = readSource(QStringLiteral(
        "src/presentation/qml/features/player/fullscreen/FullscreenControls.qml"));

    QVERIFY(!screen.isEmpty());
    QVERIFY(!utility.isEmpty());
    QVERIFY(!gesture.isEmpty());
    QVERIFY(!controls.isEmpty());

    QVERIFY(screen.contains(QStringLiteral("property bool fullScreen: false")));
    QVERIFY(screen.contains(QStringLiteral("signal fullscreenToggleRequested()")));
    QVERIFY(screen.contains(QStringLiteral("FullscreenGestureLayer {")));
    QVERIFY(screen.contains(QStringLiteral("parent: videoViewport")));
    QVERIFY(screen.contains(QStringLiteral("PlayerUtilityControls {")));
    QVERIFY(screen.contains(QStringLiteral("fullScreen: root.fullScreen")));
    QVERIFY(screen.contains(QStringLiteral(
        "onToggleFullscreenRequested: root.fullscreenToggleRequested()")));

    QVERIFY(utility.contains(QStringLiteral("FullscreenControls {")));
    QVERIFY(utility.contains(QStringLiteral("fullScreen: root.fullScreen")));
    QVERIFY(utility.contains(QStringLiteral(
        "onToggleFullscreenRequested: root.toggleFullscreenRequested()")));

    QVERIFY(gesture.contains(QStringLiteral("TapHandler {")));
    QVERIFY(gesture.contains(QStringLiteral("acceptedButtons: Qt.LeftButton")));
    QVERIFY(gesture.contains(QStringLiteral("onDoubleTapped: root.toggleFullscreenRequested()")));

    QVERIFY(controls.contains(QStringLiteral("iconId: \"fullscreen\"")));
    QVERIFY(controls.contains(QStringLiteral("qsTr(\"Exit Fullscreen\")")));
    QVERIFY(controls.contains(QStringLiteral("qsTr(\"Enter Fullscreen\")")));
    QVERIFY(!controls.contains(QStringLiteral("exit-fullscreen")));
}

void FullscreenControlsTest::fullscreenPresentationMatchesCanonicalFigmaGeometry()
{
    const QString sizes = readSource(QStringLiteral(
        "src/presentation/qml/theme/SizePrimitives.qml"));
    const QString layout = readSource(QStringLiteral(
        "src/presentation/qml/theme/LayoutTokens.qml"));
    const QString header = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/fullscreen/FullscreenHeader.qml"));
    const QString osc = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerOscLayout.qml"));
    const QString surface = readSource(QStringLiteral(
        "src/presentation/qml/surfaces/OscSurface.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));

    QVERIFY(sizes.contains(QStringLiteral("readonly property int size440: 440")));
    QVERIFY(sizes.contains(QStringLiteral("readonly property int size828: 828")));
    QVERIFY(layout.contains(QStringLiteral(
        "readonly property int fullscreenHeaderWidth: SizePrimitives.size440")));
    QVERIFY(layout.contains(QStringLiteral(
        "readonly property int oscMaximumWidthCompact: SizePrimitives.size828")));

    QVERIFY(header.contains(QStringLiteral("LayoutTokens.fullscreenHeaderWidth")));
    QVERIFY(header.contains(QStringLiteral("LayoutTokens.headerHeightCompact")));
    QVERIFY(header.contains(QStringLiteral("RadiusTokens.headerCompact")));
    QVERIFY(header.contains(QStringLiteral("MaterialTokens.headerCompactFillAlpha")));
    QVERIFY(header.contains(QStringLiteral("MaterialTokens.headerCompactBlur")));
    QVERIFY(header.contains(QStringLiteral("TitleText.MediaCompact")));
    QVERIFY(header.contains(QStringLiteral("TypographyTokens.keycapCompact")));
    QVERIFY(header.contains(QStringLiteral("qsTr(\"ESC\")")));

    QVERIFY(osc.contains(QStringLiteral("LayoutTokens.oscMaximumWidthCompact")));
    QVERIFY(surface.contains(QStringLiteral("LayoutTokens.oscMaximumWidthCompact")));
    QVERIFY(screen.contains(QStringLiteral("readonly property bool oscCompact: root.fullScreen")));
    QVERIFY(screen.contains(QStringLiteral("visible: !root.fullScreen")));
    QVERIFY(screen.contains(QStringLiteral("FullscreenHeader {")));
    QVERIFY(screen.contains(QStringLiteral("active: root.fullScreen")));
    QVERIFY(screen.contains(QStringLiteral("compact: root.oscCompact")));
}

void FullscreenControlsTest::sharedControlsAdaptWithoutDuplicatingPlaybackState()
{
    const QString iconButton = readSource(QStringLiteral(
        "src/presentation/qml/controls/buttons/IconButton.qml"));
    const QString transport = readSource(QStringLiteral(
        "src/presentation/qml/features/player/transport/TransportControls.qml"));
    const QString volume = readSource(QStringLiteral(
        "src/presentation/qml/features/player/volume/VolumeControls.qml"));
    const QString controls = readSource(QStringLiteral(
        "src/presentation/qml/features/player/fullscreen/FullscreenControls.qml"));
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/OscControlRow.qml"));

    QVERIFY(iconButton.contains(QStringLiteral("property int iconSizeOverride: 0")));
    QVERIFY(iconButton.contains(QStringLiteral("root.iconSizeOverride > 0")));

    QVERIFY(transport.contains(QStringLiteral("property bool compact: false")));
    QVERIFY(transport.contains(QStringLiteral("SpacingTokens.fullscreenTransportGap")));
    QVERIFY(transport.contains(QStringLiteral("LayoutTokens.controlIconCompact")));
    QVERIFY(transport.contains(QStringLiteral("IconButton.Secondary")));
    QVERIFY(transport.contains(QStringLiteral("IconButton.Primary")));

    QVERIFY(volume.contains(QStringLiteral(
        "iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0")));
    QVERIFY(volume.contains(QStringLiteral("visible: !root.compact")));
    QVERIFY(row.contains(QStringLiteral("readonly property int groupGap: SpacingTokens.controlGroup")));

    const QStringList featureSources{transport, volume, controls};
    for (const QString& source : featureSources) {
        verifyAbsent(source,
                     {QStringLiteral("PlaybackSession"),
                      QStringLiteral("PlaybackCommandBus"),
                      QStringLiteral("libmpv"),
                      QStringLiteral("mpv_")});
    }
}

void FullscreenControlsTest::platformSpecificWindowWorkRemainsDeferred()
{
    const QString controller = readSource(QStringLiteral(
        "src/presentation/qml/shell/window/FullscreenWindowController.qml"));
    const QString window = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    const QString gesture = readSource(QStringLiteral(
        "src/presentation/qml/features/player/fullscreen/FullscreenGestureLayer.qml"));

    const QString combined = controller + window + gesture;
    verifyAbsent(combined,
                 {QStringLiteral("FramelessWindowHint"),
                  QStringLiteral("nativeEvent"),
                  QStringLiteral("WM_NCHITTEST"),
                  QStringLiteral("Dwm"),
                  QStringLiteral("SetWindowLong"),
                  QStringLiteral("Timer {")});
}

} // namespace player::presentation::qml

QTEST_GUILESS_MAIN(player::presentation::qml::FullscreenControlsTest)
#include "fullscreen_controls_test.moc"
