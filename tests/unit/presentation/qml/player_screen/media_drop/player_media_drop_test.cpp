#include <QCoreApplication>
#include <QFile>
#include <QString>
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

class PlayerMediaDropTest final : public QObject
{
    Q_OBJECT

private slots:
    void dropOverlayOwnsDropAreaOnly();
    void dropRouteUsesInjectedHandler();
    void dragStateJoinsCursorLock();
};

void PlayerMediaDropTest::dropOverlayOwnsDropAreaOnly()
{
    const QString path = QStringLiteral(
        "src/presentation/qml/screens/player/overlays/drop/PlayerDropOverlay.qml");
    const QString source = readSource(path);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(path)));

    QVERIFY(source.contains(QStringLiteral("DropArea {")));
    QVERIFY(source.contains(QStringLiteral("drag.hasUrls")));
    QVERIFY(source.contains(QStringLiteral("drop.urls")));
    QVERIFY(source.contains(QStringLiteral("canHandle(drag.urls)")));
    QVERIFY(source.contains(QStringLiteral("handleDrop(drop.urls)")));
    QVERIFY(source.contains(QStringLiteral("drop.acceptProposedAction()")));
    QVERIFY(source.contains(QStringLiteral("drop.accepted = false")));
    QVERIFY(!source.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!source.contains(QStringLiteral("CommandBus")));
    QVERIFY(!source.contains(QStringLiteral("mpv")));
    QVERIFY(!source.contains(QStringLiteral("Timer")));
}

void PlayerMediaDropTest::dropRouteUsesInjectedHandler()
{
    const QString bootstrap = readSource(QStringLiteral(
        "src/app/bootstrap/application_bootstrap.cpp"));
    const QString mainWindow = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));

    QVERIFY(bootstrap.contains(QStringLiteral("mediaDropHandler")));
    QVERIFY(mainWindow.contains(QStringLiteral("property var mediaDropHandler: null")));
    QVERIFY(mainWindow.contains(QStringLiteral("mediaDropHandler: window.mediaDropHandler")));
    QVERIFY(screen.contains(QStringLiteral("property var mediaDropHandler: null")));
    QVERIFY(screen.contains(QStringLiteral("PlayerDropOverlay {")));
    QVERIFY(screen.contains(QStringLiteral("mediaDropHandler: root.mediaDropHandler")));
}

void PlayerMediaDropTest::dragStateJoinsCursorLock()
{
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    QVERIFY(screen.contains(QStringLiteral("|| dropOverlay.dragActive")));
    QVERIFY(screen.contains(QStringLiteral("dragActive: root.controlsDragActive")));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    player::presentation::qml::PlayerMediaDropTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "player_media_drop_test.moc"
