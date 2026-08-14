#include <QCoreApplication>
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

} // namespace

class PlayerTopRegionTest final : public QObject
{
    Q_OBJECT

private slots:
    void screenInjectsHeaderThroughTopRegionHost();
    void headerKeepsPodsIndependent();
    void mediaInfoPrioritizesTitle();
    void windowActionsEmitIntentsOnly();
    void shellOwnsGenericWindowCommands();
};

void PlayerTopRegionTest::screenInjectsHeaderThroughTopRegionHost()
{
    const QString path =
        QStringLiteral("src/presentation/qml/screens/player/PlayerScreen.qml");
    const QString source = readSource(path);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(path)));

    QVERIFY(source.contains(QStringLiteral("PlayerTopRegion {")));
    QVERIFY(source.contains(QStringLiteral("PlayerFloatingHeader {")));
    QVERIFY(source.contains(QStringLiteral("property string mediaTitle: \"\"")));
    QVERIFY(source.contains(QStringLiteral("property string mediaMetadataText: \"\"")));
    QVERIFY(source.contains(QStringLiteral("property bool windowExpanded: false")));
    QVERIFY(source.contains(QStringLiteral("signal minimizeRequested()")));
    QVERIFY(source.contains(QStringLiteral("signal maximizeRestoreRequested()")));
    QVERIFY(source.contains(QStringLiteral("signal closeRequested()")));

    const QStringList forbidden{
        QStringLiteral("IconButton"),
        QStringLiteral("TitleText"),
        QStringLiteral("CaptionText"),
        QStringLiteral("PlaybackSession"),
        QStringLiteral("libmpv"),
        QStringLiteral("mpv_")};
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

void PlayerTopRegionTest::headerKeepsPodsIndependent()
{
    const QString path = QStringLiteral(
        "src/presentation/qml/features/player/header/PlayerFloatingHeader.qml");
    const QString source = readSource(path);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(path)));

    QVERIFY(source.contains(QStringLiteral("WindowActionsPod {")));
    QVERIFY(source.contains(QStringLiteral("MediaInfoPod {")));
    QVERIFY(source.contains(QStringLiteral("right: parent.right")));
    QVERIFY(source.contains(QStringLiteral("left: parent.left")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerInfoWidth")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerInfoWidthCompact")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerActionsWidth")));
    QVERIFY(source.contains(QStringLiteral("height: root.podHeight")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.controlAdjacent")));

    const QStringList forbidden{
        QStringLiteral("IconButton"),
        QStringLiteral("TitleText"),
        QStringLiteral("CaptionText"),
        QStringLiteral("PlaybackSession"),
        QStringLiteral("libmpv"),
        QStringLiteral("mpv_")};
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

void PlayerTopRegionTest::mediaInfoPrioritizesTitle()
{
    const QString path = QStringLiteral(
        "src/presentation/qml/features/player/header/MediaInfoPod.qml");
    const QString source = readSource(path);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(path)));

    QVERIFY(source.contains(
        QStringLiteral("visible: root.mediaTitle.trim().length > 0")));
    QVERIFY(source.contains(QStringLiteral("TitleText {")));
    QVERIFY(source.contains(QStringLiteral("CaptionText {")));
    QVERIFY(source.contains(QStringLiteral("metadataVisible")));
    QVERIFY(source.contains(QStringLiteral("!root.compact")));
    QVERIFY(source.contains(QStringLiteral("root.width >= LayoutTokens.headerInfoWidth")));
    QVERIFY(source.contains(QStringLiteral("MaterialTokens.headerFillAlpha")));
    QVERIFY(source.contains(QStringLiteral("MaterialTokens.headerCompactFillAlpha")));
    QVERIFY(source.contains(QStringLiteral("RadiusTokens.header")));
    QVERIFY(source.contains(QStringLiteral("RadiusTokens.headerCompact")));

    const QString titlePath = QStringLiteral(
        "src/presentation/qml/primitives/text/TitleText.qml");
    const QString titleSource = readSource(titlePath);
    QVERIFY2(!titleSource.isEmpty(), qPrintable(sourcePath(titlePath)));
    QVERIFY(titleSource.contains(QStringLiteral("wrapMode: Text.NoWrap")));
    QVERIFY(titleSource.contains(QStringLiteral("elide: Text.ElideRight")));
    QVERIFY(titleSource.contains(QStringLiteral("maximumLineCount: 1")));
}

void PlayerTopRegionTest::windowActionsEmitIntentsOnly()
{
    const QString path = QStringLiteral(
        "src/presentation/qml/features/player/header/WindowActionsPod.qml");
    const QString source = readSource(path);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(path)));

    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerActionsWidth")));
    QVERIFY(source.contains(QStringLiteral("iconId: \"minimize\"")));
    QVERIFY(source.contains(
        QStringLiteral("root.windowExpanded ? \"restore\" : \"maximize\"")));
    QVERIFY(source.contains(QStringLiteral("iconId: \"close\"")));
    QVERIFY(source.contains(QStringLiteral("signal minimizeRequested()")));
    QVERIFY(source.contains(QStringLiteral("signal maximizeRestoreRequested()")));
    QVERIFY(source.contains(QStringLiteral("signal closeRequested()")));

    const QStringList forbidden{
        QStringLiteral("showMinimized"),
        QStringLiteral("showMaximized"),
        QStringLiteral("showNormal"),
        QStringLiteral("close()"),
        QStringLiteral("Window.window"),
        QStringLiteral("FramelessWindowHint"),
        QStringLiteral("startSystemMove"),
        QStringLiteral("Win32")};
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

void PlayerTopRegionTest::shellOwnsGenericWindowCommands()
{
    const QString path =
        QStringLiteral("src/presentation/qml/shell/MainWindow.qml");
    const QString source = readSource(path);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(path)));

    QVERIFY(source.contains(QStringLiteral("Window.Maximized")));
    QVERIFY(source.contains(QStringLiteral("Window.FullScreen")));
    QVERIFY(source.contains(QStringLiteral("window.showMinimized()")));
    QVERIFY(source.contains(QStringLiteral("window.showMaximized()")));
    QVERIFY(source.contains(QStringLiteral("window.showNormal()")));
    QVERIFY(source.contains(QStringLiteral("window.close()")));
    QVERIFY(source.contains(QStringLiteral("title: qsTr(\"Player\")")));
    QVERIFY(source.contains(QStringLiteral("color: ColorTokens.surfaceCanvas")));
    QVERIFY(!source.contains(QStringLiteral("Qt.FramelessWindowHint")));
    QVERIFY(!source.contains(QStringLiteral("startSystemMove")));
    QVERIFY(!source.contains(QStringLiteral("Win32")));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    player::presentation::qml::PlayerTopRegionTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "player_top_region_test.moc"
