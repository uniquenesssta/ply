#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QMetaObject>
#include <QQuickItem>
#include <QQuickView>
#include <QQmlError>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>
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

QString viewDiagnostics(const QQuickView& view)
{
    QStringList diagnostics;
    diagnostics.append(QStringLiteral("status=%1").arg(static_cast<int>(view.status())));
    for (const QQmlError& error : view.errors()) {
        diagnostics.append(error.toString());
    }
    return diagnostics.join(QLatin1Char('\n'));
}

QQuickItem* createCursorController(QQuickView& view)
{
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.resize(640, 360);
    view.setSource(QUrl::fromLocalFile(sourcePath(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerCursorVisibilityController.qml"))));

    if (view.status() != QQuickView::Ready || view.rootObject() == nullptr) {
        return nullptr;
    }

    view.show();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    return view.rootObject();
}

bool notifyActivity(QQuickItem* controller, const QString& reason)
{
    return QMetaObject::invokeMethod(
        controller,
        "notifyActivity",
        Q_ARG(QVariant, QVariant(reason)));
}

} // namespace

class PlayerCursorVisibilityTest final : public QObject
{
    Q_OBJECT

private slots:
    void cursorFollowsOscVisibilityAndInteractionLocks();
    void screenIntegratesCursorPolicyWithoutSecondTimer();
};

void PlayerCursorVisibilityTest::cursorFollowsOscVisibilityAndInteractionLocks()
{
    QQuickView view;
    QQuickItem* controller = createCursorController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));

    QVERIFY(!controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("playing", true));
    QVERIFY(controller->setProperty("pointerInside", true));
    QVERIFY(notifyActivity(controller, QStringLiteral("test-setup")));
    QVERIFY(!controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("oscVisible", false));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("scrubbing", true));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());
    QVERIFY(controller->setProperty("scrubbing", false));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("popupOpen", true));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());
    QVERIFY(controller->setProperty("popupOpen", false));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("errorVisible", true));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());
    QVERIFY(controller->setProperty("errorVisible", false));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("cursorHideSuppressed", true));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());
    QVERIFY(controller->setProperty("cursorHideSuppressed", false));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("oscVisible", true));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("oscVisible", false));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());
    QVERIFY(controller->setProperty("playing", false));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());
}

void PlayerCursorVisibilityTest::screenIntegratesCursorPolicyWithoutSecondTimer()
{
    const QString chromeController = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeVisibilityController.qml"));
    const QString cursorController = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerCursorVisibilityController.qml"));
    const QString activityLayer = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeActivityLayer.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));

    QVERIFY(!chromeController.isEmpty());
    QVERIFY(!cursorController.isEmpty());
    QVERIFY(!activityLayer.isEmpty());
    QVERIFY(!screen.isEmpty());

    QCOMPARE(chromeController.count(QStringLiteral("Timer {")), 1);
    QVERIFY(!cursorController.contains(QStringLiteral("Timer {")));
    QVERIFY(!activityLayer.contains(QStringLiteral("Timer {")));

    QVERIFY(cursorController.contains(QStringLiteral(
        "readonly property bool cursorHidden: root.shouldHideCursor()")));
    QVERIFY(cursorController.contains(QStringLiteral("&& !root.oscVisible")));
    QVERIFY(cursorController.contains(QStringLiteral("&& !root.scrubbing")));
    QVERIFY(cursorController.contains(QStringLiteral("&& !root.popupOpen")));
    QVERIFY(cursorController.contains(QStringLiteral("&& !root.errorVisible")));
    QVERIFY(cursorController.contains(QStringLiteral("&& !root.cursorHideSuppressed")));
    QVERIFY(cursorController.contains(QStringLiteral("R6-15 cursor visibility")));
    QVERIFY(!cursorController.contains(QStringLiteral("oscHideDelay")));
    QVERIFY(!cursorController.contains(QStringLiteral("oscFullscreenHideDelay")));

    QVERIFY(activityLayer.contains(QStringLiteral("property bool cursorHidden: false")));
    QVERIFY(activityLayer.contains(QStringLiteral(
        "cursorShape: root.cursorHidden ? Qt.BlankCursor : undefined")));
    QVERIFY(activityLayer.contains(QStringLiteral("onPointChanged:")));

    QVERIFY(screen.contains(QStringLiteral("PlayerCursorVisibilityController {")));
    QVERIFY(screen.contains(QStringLiteral("oscVisible: root.oscVisible")));
    QVERIFY(screen.contains(QStringLiteral("scrubbing: root.timelineInteractionActive")));
    QVERIFY(screen.contains(QStringLiteral("popupOpen: root.popupOpen")));
    QVERIFY(screen.contains(QStringLiteral("errorVisible: root.errorOverlayVisible")));
    QVERIFY(screen.contains(QStringLiteral(
        "cursorHideSuppressed: root.cursorHideSuppressed")));
    QVERIFY(screen.contains(QStringLiteral(
        "readonly property bool cursorHidden: cursorVisibilityController.cursorHidden")));
    QVERIFY(screen.contains(QStringLiteral(
        "cursorHidden: root.cursorHidden")));

    const QString combined = cursorController + activityLayer + screen;
    QVERIFY(!combined.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!combined.contains(QStringLiteral("PlaybackCommandBus")));
    QVERIFY(!combined.contains(QStringLiteral("libmpv")));
    QVERIFY(!combined.contains(QStringLiteral("mpv_")));
    QVERIFY(!cursorController.contains(QStringLiteral("PlayerStatusOverlay")));
    QVERIFY(!activityLayer.contains(QStringLiteral("PlayerStatusOverlay")));
    QVERIFY(screen.contains(QStringLiteral("PlayerStatusOverlay {")));
    QVERIFY(!combined.contains(QStringLiteral("HudMessageQueue")));
}

} // namespace player::presentation::qml

QTEST_MAIN(player::presentation::qml::PlayerCursorVisibilityTest)
#include "player_cursor_visibility_test.moc"
