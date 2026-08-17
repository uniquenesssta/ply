#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
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
    void screenInjectsCanonicalHeaderThroughTopRegionHost();
    void headerMatchesCanonicalFigmaStructure();
    void mediaInfoProjectionFeedsHeader();
    void obsoleteHeaderPodsAreRemoved();
    void windowActionGlyphsMatchCanonicalFigmaGeometry();
    void shellOwnsGenericWindowCommands();
};

void PlayerTopRegionTest::screenInjectsCanonicalHeaderThroughTopRegionHost()
{
    const QString path =
        QStringLiteral("src/presentation/qml/screens/player/PlayerScreen.qml");
    const QString source = readSource(path);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(path)));

    QVERIFY(source.contains(QStringLiteral("PlayerTopRegion {")));
    QVERIFY(source.contains(QStringLiteral("PlayerFloatingHeader {")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerWidth")));
    QVERIFY(source.contains(QStringLiteral("resolvedMediaTitle")));
    QVERIFY(source.contains(QStringLiteral("resolvedMediaMetadataText")));
    QVERIFY(source.contains(QStringLiteral("playbackStatusText")));
    QVERIFY(source.contains(QStringLiteral("horizontalCenter: parent.horizontalCenter")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.floatingTop")));
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

void PlayerTopRegionTest::headerMatchesCanonicalFigmaStructure()
{
    const QString path = QStringLiteral(
        "src/presentation/qml/screens/player/header/PlayerFloatingHeader.qml");
    const QString source = readSource(path);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(path)));

    QVERIFY(source.contains(QStringLiteral("Panel {")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerWidth")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerHeight")));
    QVERIFY(source.contains(QStringLiteral("RadiusTokens.header")));
    QVERIFY(source.contains(QStringLiteral("MaterialTokens.headerFillAlpha")));
    QVERIFY(source.contains(QStringLiteral("MaterialTokens.headerBlur")));
    QVERIFY(source.contains(QStringLiteral("TitleText {")));
    QVERIFY(source.contains(QStringLiteral("CaptionText {")));
    QVERIFY(source.contains(QStringLiteral("property string statusText: \"\"")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerStatusX")));
    QVERIFY(source.contains(QStringLiteral("iconId: \"close\"")));
    QVERIFY(source.contains(QStringLiteral("objectName: \"playerWindowCloseButton\"")));
    QVERIFY(!source.contains(QStringLiteral("WindowActionsPod {")));
    QVERIFY(!source.contains(QStringLiteral("MediaInfoPod {")));
    QVERIFY(!source.contains(QStringLiteral("iconId: \"minimize\"")));
    QVERIFY(!source.contains(QStringLiteral("iconId: \"maximize\"")));
}

void PlayerTopRegionTest::mediaInfoProjectionFeedsHeader()
{
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString mediaHeader = readSource(QStringLiteral(
        "src/presentation/viewmodels/player/media/player_media_view_model.h"));

    QVERIFY(screen.contains(QStringLiteral("root.mediaViewModel.title")));
    QVERIFY(screen.contains(QStringLiteral("root.mediaViewModel.metadataText")));
    QVERIFY(mediaHeader.contains(QStringLiteral("Q_PROPERTY(QString title")));
    QVERIFY(mediaHeader.contains(QStringLiteral("Q_PROPERTY(QString metadataText")));
}

void PlayerTopRegionTest::obsoleteHeaderPodsAreRemoved()
{
    QVERIFY(!QFileInfo::exists(sourcePath(QStringLiteral(
        "src/presentation/qml/screens/player/header/MediaInfoPod.qml"))));
    QVERIFY(!QFileInfo::exists(sourcePath(QStringLiteral(
        "src/presentation/qml/screens/player/header/WindowActionsPod.qml"))));
}

void PlayerTopRegionTest::windowActionGlyphsMatchCanonicalFigmaGeometry()
{
    const QString minimize = readSource(
        QStringLiteral("src/presentation/qml/assets/icons/minimize.svg"));
    const QString maximize = readSource(
        QStringLiteral("src/presentation/qml/assets/icons/maximize.svg"));
    const QString close = readSource(
        QStringLiteral("src/presentation/qml/assets/icons/close.svg"));

    QVERIFY(minimize.contains(QStringLiteral(
        "x1=\"7.75\" y1=\"12.25\" x2=\"14.25\" y2=\"12.25\"")));
    QVERIFY(maximize.contains(QStringLiteral(
        "x=\"7.75\" y=\"7.75\" width=\"6.5\" height=\"6.5\" rx=\"0.75\"")));
    QVERIFY(close.contains(QStringLiteral("d=\"M7 7L15 15M15 7L7 15\"")));

    const QStringList icons{minimize, maximize, close};
    for (const QString& icon : icons) {
        QVERIFY(icon.contains(QStringLiteral("width=\"22\" height=\"22\"")));
        QVERIFY(icon.contains(QStringLiteral("stroke=\"#76707B\"")));
        QVERIFY(icon.contains(QStringLiteral("stroke-width=\"1.5\"")));
        QVERIFY(!icon.contains(QStringLiteral("opacity=")));
    }

    QVERIFY(!QFileInfo::exists(sourcePath(
        QStringLiteral("src/presentation/qml/assets/icons/restore.svg"))));
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
