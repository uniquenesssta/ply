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

class VideoViewportTest final : public QObject
{
    Q_OBJECT

private slots:
    void exposesMediaCapabilityContract();
    void usesSemanticBackgroundsForAllMediaStates();
    void keepsRenderItemInsideDedicatedVideoSurface();
};

void VideoViewportTest::exposesMediaCapabilityContract()
{
    const QString relativePath = QStringLiteral(
        "src/presentation/qml/screens/player/layout/VideoViewport.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    QVERIFY(source.contains(QStringLiteral("property bool hasMedia: false")));
    QVERIFY(source.contains(QStringLiteral("property bool hasVideo: false")));
    QVERIFY(source.contains(QStringLiteral("readonly property bool videoVisible")));
    QVERIFY(source.contains(QStringLiteral("root.hasMedia && root.hasVideo")));
    QVERIFY(source.contains(QStringLiteral("readonly property bool audioOnly")));
    QVERIFY(source.contains(QStringLiteral("root.hasMedia && !root.hasVideo")));
}

void VideoViewportTest::usesSemanticBackgroundsForAllMediaStates()
{
    const QString relativePath = QStringLiteral(
        "src/presentation/qml/screens/player/layout/VideoViewport.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    const QStringList expectedTokens{
        QStringLiteral("ColorTokens.surfaceEmpty"),
        QStringLiteral("ColorTokens.surfaceAudio"),
        QStringLiteral("ColorTokens.surfaceLetterbox")};
    for (const QString& token : expectedTokens) {
        QVERIFY2(source.contains(token), qPrintable(token));
    }

    QVERIFY(source.contains(
        QStringLiteral("objectName: \"playerVideoViewportBackground\"")));
    QVERIFY(source.contains(QStringLiteral("color: root.backgroundColor")));
    QVERIFY(!source.contains(QLatin1Char('#')));
}

void VideoViewportTest::keepsRenderItemInsideDedicatedVideoSurface()
{
    const QString viewportPath = QStringLiteral(
        "src/presentation/qml/screens/player/layout/VideoViewport.qml");
    const QString viewport = readSource(viewportPath);
    QVERIFY2(!viewport.isEmpty(), qPrintable(sourcePath(viewportPath)));

    QVERIFY(viewport.contains(QStringLiteral("VideoSurface {")));
    QVERIFY(viewport.contains(QStringLiteral("anchors.fill: parent")));
    QVERIFY(viewport.contains(QStringLiteral("visible: root.videoVisible")));
    QVERIFY(!viewport.contains(QStringLiteral("MpvVideoItem")));
    QVERIFY(!viewport.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!viewport.contains(QStringLiteral("libmpv")));
    QVERIFY(!viewport.contains(QStringLiteral("mpv_")));

    const QString surfacePath = QStringLiteral(
        "src/presentation/qml/features/player/video/VideoSurface.qml");
    const QString surface = readSource(surfacePath);
    QVERIFY2(!surface.isEmpty(), qPrintable(sourcePath(surfacePath)));

    QVERIFY(surface.contains(QStringLiteral("MpvVideoItem {")));
    QVERIFY(surface.contains(QStringLiteral("anchors.fill: parent")));
    QVERIFY(surface.contains(QStringLiteral("ColorTokens.surfaceLetterbox")));
    QVERIFY(!surface.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!surface.contains(QStringLiteral("libmpv")));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    player::presentation::qml::VideoViewportTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "video_viewport_test.moc"
