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

class PlayerScreenStructureTest final : public QObject
{
    Q_OBJECT

private slots:
    void screenComposesDedicatedHosts();
    void hostsKeepLayerResponsibilitiesSeparate();
    void legacyCombinedChromeIsRemoved();
};

void PlayerScreenStructureTest::screenComposesDedicatedHosts()
{
    const QString relativePath =
        QStringLiteral("src/presentation/qml/screens/player/PlayerScreen.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    const QStringList expectedTypes{
        QStringLiteral("VideoViewport {"),
        QStringLiteral("PlayerTopRegion {"),
        QStringLiteral("PlayerBottomRegion {"),
        QStringLiteral("PlayerOverlayStack {"),
        QStringLiteral("PlayerDrawerHost {")};
    for (const QString& type : expectedTypes) {
        QVERIFY2(source.contains(type), qPrintable(type));
    }

    const QStringList forbiddenImplementation{
        QStringLiteral("VideoSurface {"),
        QStringLiteral("MpvVideoItem"),
        QStringLiteral("Rectangle {"),
        QStringLiteral("Text {"),
        QStringLiteral("IconButton"),
        QStringLiteral("PlaybackSession"),
        QStringLiteral("libmpv"),
        QStringLiteral("mpv_")};
    for (const QString& token : forbiddenImplementation) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }

    QVERIFY(source.contains(QStringLiteral("SpacingTokens.floatingTop")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.floatingEdge")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscBottom")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.inspectorEdge")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.inspectorRight")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.windowSafeMinimum")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.headerHeight")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscHeight")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.inspectorWidth")));
}

void PlayerScreenStructureTest::hostsKeepLayerResponsibilitiesSeparate()
{
    struct HostExpectation {
        const char* relativePath;
        const char* objectName;
        const char* zToken;
    };

    const HostExpectation hosts[] = {
        {"src/presentation/qml/screens/player/layout/PlayerTopRegion.qml",
         "playerTopRegion", "ZOrderTokens.floatingHeader"},
        {"src/presentation/qml/screens/player/layout/PlayerBottomRegion.qml",
         "playerBottomRegion", "ZOrderTokens.osc"},
        {"src/presentation/qml/screens/player/overlays/PlayerOverlayStack.qml",
         "playerOverlayStack", "ZOrderTokens.overlay"},
        {"src/presentation/qml/screens/player/drawers/PlayerDrawerHost.qml",
         "playerDrawerHost", "ZOrderTokens.inspector"}};

    for (const HostExpectation& host : hosts) {
        const QString relativePath = QString::fromLatin1(host.relativePath);
        const QString source = readSource(relativePath);
        QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));
        QVERIFY(source.contains(
            QStringLiteral("objectName: \"")
            + QString::fromLatin1(host.objectName)
            + QStringLiteral("\"")));
        QVERIFY(source.contains(QString::fromLatin1(host.zToken)));
        QVERIFY(source.contains(QStringLiteral("default property alias content")));
        QVERIFY(source.contains(QStringLiteral("readonly property Item contentItem")));

        const QStringList forbidden{
            QStringLiteral("PlaybackSession"),
            QStringLiteral("libmpv"),
            QStringLiteral("mpv_"),
            QStringLiteral("Rectangle {"),
            QStringLiteral("Text {")};
        for (const QString& token : forbidden) {
            QVERIFY2(!source.contains(token), qPrintable(token));
        }
    }

    const QString viewportPath =
        QStringLiteral("src/presentation/qml/screens/player/layout/VideoViewport.qml");
    const QString viewport = readSource(viewportPath);
    QVERIFY2(!viewport.isEmpty(), qPrintable(sourcePath(viewportPath)));
    QVERIFY(viewport.contains(QStringLiteral("objectName: \"playerVideoViewport\"")));
    QVERIFY(viewport.contains(QStringLiteral("ZOrderTokens.video")));
    QVERIFY(viewport.contains(QStringLiteral("VideoSurface {")));
    QVERIFY(viewport.contains(QStringLiteral("anchors.fill: parent")));
    QVERIFY(!viewport.contains(QStringLiteral("MpvVideoItem")));
}

void PlayerScreenStructureTest::legacyCombinedChromeIsRemoved()
{
    const QString legacyPath = QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChrome.qml");
    QVERIFY2(!QFileInfo::exists(sourcePath(legacyPath)), qPrintable(sourcePath(legacyPath)));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    player::presentation::qml::PlayerScreenStructureTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "player_screen_structure_test.moc"
