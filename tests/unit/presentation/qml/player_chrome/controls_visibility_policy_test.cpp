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

constexpr int kWindowHideDelayMs = 2200;
constexpr int kFullscreenHideDelayMs = 1600;

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

QQuickItem* createVisibilityController(QQuickView& view)
{
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.resize(640, 360);
    view.setSource(QUrl::fromLocalFile(sourcePath(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeVisibilityController.qml"))));

    if (view.status() != QQuickView::Ready || view.rootObject() == nullptr) {
        return nullptr;
    }

    view.show();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    return view.rootObject();
}

QObject* findInactivityTimer(QObject* controller)
{
    return controller->findChild<QObject*>(QStringLiteral("oscInactivityTimer"));
}

bool invokeNoArg(QObject* object, const char* method)
{
    return QMetaObject::invokeMethod(object, method, Qt::DirectConnection);
}

bool invokeActivity(QObject* object, const QString& reason)
{
    return QMetaObject::invokeMethod(
        object,
        "notifyActivity",
        Qt::DirectConnection,
        Q_ARG(QVariant, QVariant(reason)));
}

} // namespace

class PlayerControlsVisibilityPolicyTest final : public QObject
{
    Q_OBJECT

private slots:
    void matureInteractionLocksKeepControlsVisible();
    void activityAndWindowModesReuseOneInactivityTimer();
    void playerScreenRoutesHoverFocusAndOverlayLocks();
};

void PlayerControlsVisibilityPolicyTest::matureInteractionLocksKeepControlsVisible()
{
    QQuickView view;
    QQuickItem* controller = createVisibilityController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));

    QObject* timer = findInactivityTimer(controller);
    QVERIFY(timer != nullptr);

    QVERIFY(controller->setProperty("playing", true));
    QVERIFY(!controller->property("visibilityLocked").toBool());
    QVERIFY(timer->property("running").toBool());

    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    const QStringList visibilityLocks{
        QStringLiteral("scrubbing"),
        QStringLiteral("controlsHovered"),
        QStringLiteral("controlsFocused"),
        QStringLiteral("popupOpen"),
        QStringLiteral("menuOpen"),
        QStringLiteral("drawerOpen"),
        QStringLiteral("modalActive"),
        QStringLiteral("errorVisible")};

    for (const QString& propertyName : visibilityLocks) {
        QVERIFY2(controller->setProperty(propertyName.toUtf8().constData(), true),
                 qPrintable(propertyName));
        QVERIFY2(controller->property("visibilityLocked").toBool(),
                 qPrintable(propertyName));
        QVERIFY2(controller->property("chromeVisible").toBool(),
                 qPrintable(propertyName));
        QVERIFY2(!timer->property("running").toBool(),
                 qPrintable(propertyName));

        QVERIFY(invokeNoArg(controller, "hideIfEligible"));
        QVERIFY2(controller->property("chromeVisible").toBool(),
                 qPrintable(propertyName));

        QVERIFY2(controller->setProperty(propertyName.toUtf8().constData(), false),
                 qPrintable(propertyName));
        QVERIFY2(!controller->property("visibilityLocked").toBool(),
                 qPrintable(propertyName));
        QVERIFY2(timer->property("running").toBool(),
                 qPrintable(propertyName));

        QVERIFY(invokeNoArg(controller, "hideIfEligible"));
        QVERIFY2(!controller->property("chromeVisible").toBool(),
                 qPrintable(propertyName));
    }

    QVERIFY(controller->setProperty("playing", false));
    QVERIFY(controller->property("visibilityLocked").toBool());
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(!timer->property("running").toBool());

    QVERIFY(controller->setProperty("playing", true));
    QVERIFY(!controller->property("visibilityLocked").toBool());
    QVERIFY(timer->property("running").toBool());
}

void PlayerControlsVisibilityPolicyTest::activityAndWindowModesReuseOneInactivityTimer()
{
    QQuickView view;
    QQuickItem* controller = createVisibilityController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));

    QObject* timer = findInactivityTimer(controller);
    QVERIFY(timer != nullptr);
    QCOMPARE(controller->findChildren<QObject*>(QStringLiteral("oscInactivityTimer")).size(), 1);

    QVERIFY(controller->setProperty("playing", true));
    QCOMPARE(timer->property("interval").toInt(), kWindowHideDelayMs);
    QVERIFY(timer->property("running").toBool());

    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(invokeActivity(controller, QStringLiteral("pointer-move")));
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(timer->property("running").toBool());

    QVERIFY(controller->setProperty("fullScreen", true));
    QCOMPARE(findInactivityTimer(controller), timer);
    QCOMPARE(timer->property("interval").toInt(), kFullscreenHideDelayMs);
    QVERIFY(timer->property("running").toBool());

    QVERIFY(controller->setProperty("fullScreen", false));
    QCOMPARE(findInactivityTimer(controller), timer);
    QCOMPARE(timer->property("interval").toInt(), kWindowHideDelayMs);
    QVERIFY(timer->property("running").toBool());

    for (int iteration = 0; iteration < 10; ++iteration) {
        QVERIFY(controller->setProperty("playing", false));
        QVERIFY(controller->property("chromeVisible").toBool());
        QVERIFY(!timer->property("running").toBool());

        QVERIFY(controller->setProperty("playing", true));
        QVERIFY(timer->property("running").toBool());
        QCOMPARE(findInactivityTimer(controller), timer);
    }

    QCOMPARE(controller->findChildren<QObject*>(QStringLiteral("oscInactivityTimer")).size(), 1);

    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());
    QVERIFY(controller->setProperty("controlsFocused", true));
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(!timer->property("running").toBool());
}

void PlayerControlsVisibilityPolicyTest::playerScreenRoutesHoverFocusAndOverlayLocks()
{
    const QString controller = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeVisibilityController.qml"));
    const QString activityLayer = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeActivityLayer.qml"));
    const QString cursorController = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerCursorVisibilityController.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString oscLayout = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerOscLayout.qml"));
    const QString topRegion = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/layout/PlayerTopRegion.qml"));

    QVERIFY(!controller.isEmpty());
    QVERIFY(!activityLayer.isEmpty());
    QVERIFY(!cursorController.isEmpty());
    QVERIFY(!screen.isEmpty());
    QVERIFY(!oscLayout.isEmpty());
    QVERIFY(!topRegion.isEmpty());

    QCOMPARE(controller.count(QStringLiteral("Timer {")), 1);
    QVERIFY(!activityLayer.contains(QStringLiteral("Timer {")));
    QVERIFY(!cursorController.contains(QStringLiteral("Timer {")));
    QVERIFY(!oscLayout.contains(QStringLiteral("Timer {")));
    QVERIFY(!topRegion.contains(QStringLiteral("Timer {")));

    const QStringList controllerInputs{
        QStringLiteral("property bool controlsHovered: false"),
        QStringLiteral("property bool controlsFocused: false"),
        QStringLiteral("property bool popupOpen: false"),
        QStringLiteral("property bool menuOpen: false"),
        QStringLiteral("property bool drawerOpen: false"),
        QStringLiteral("property bool modalActive: false"),
        QStringLiteral("property bool errorVisible: false")};
    for (const QString& input : controllerInputs) {
        QVERIFY2(controller.contains(input), qPrintable(input));
    }

    QVERIFY(controller.contains(QStringLiteral("root.controlsHovered")));
    QVERIFY(controller.contains(QStringLiteral("root.controlsFocused")));
    QVERIFY(controller.contains(QStringLiteral("root.menuOpen")));
    QVERIFY(controller.contains(QStringLiteral("root.drawerOpen")));
    QVERIFY(controller.contains(QStringLiteral("root.modalActive")));
    QVERIFY(controller.contains(QStringLiteral("onControlsHoveredChanged:")));
    QVERIFY(controller.contains(QStringLiteral("onControlsFocusedChanged:")));
    QVERIFY(controller.contains(QStringLiteral("onMenuOpenChanged:")));
    QVERIFY(controller.contains(QStringLiteral("onDrawerOpenChanged:")));
    QVERIFY(controller.contains(QStringLiteral("onModalActiveChanged:")));

    QVERIFY(oscLayout.contains(QStringLiteral("FocusScope {")));
    QVERIFY(oscLayout.contains(QStringLiteral(
        "readonly property bool controlsHovered: surfaceHover.hovered")));
    QVERIFY(oscLayout.contains(QStringLiteral(
        "readonly property bool controlsFocused: root.activeFocus")));
    QVERIFY(oscLayout.contains(QStringLiteral("HoverHandler {")));
    QVERIFY(oscLayout.contains(QStringLiteral(
        "objectName: \"playerOscSurfaceHoverHandler\"")));

    QVERIFY(topRegion.contains(QStringLiteral("FocusScope {")));
    QVERIFY(topRegion.contains(QStringLiteral(
        "readonly property bool controlsFocused: root.activeFocus")));

    QVERIFY(screen.contains(QStringLiteral("property bool menuOpen: false")));
    QVERIFY(screen.contains(QStringLiteral("property bool drawerOpen: false")));
    QVERIFY(screen.contains(QStringLiteral("property bool playlistDrawerOpen: false")));
    QVERIFY(screen.contains(QStringLiteral("property bool modalActive: false")));
    QVERIFY(screen.contains(QStringLiteral(
        "readonly property bool anyDrawerOpen: root.drawerOpen || root.playlistDrawerOpen")));
    QVERIFY(screen.contains(QStringLiteral(
        "readonly property bool chromeControlsHovered: playerOscLayout.controlsHovered")));
    QVERIFY(screen.contains(QStringLiteral("topRegion.controlsFocused")));
    QVERIFY(screen.contains(QStringLiteral("playerOscLayout.controlsFocused")));
    QVERIFY(screen.contains(QStringLiteral("controlsHovered: root.chromeControlsHovered")));
    QVERIFY(screen.contains(QStringLiteral("controlsFocused: root.chromeControlsFocused")));
    QVERIFY(screen.contains(QStringLiteral("menuOpen: root.menuOpen")));
    QCOMPARE(screen.count(QStringLiteral("drawerOpen: root.anyDrawerOpen")), 2);
    QVERIFY(screen.contains(QStringLiteral("modalActive: root.modalActive")));
    QVERIFY(screen.contains(QStringLiteral("id: playerOscLayout")));

    const QString combined = controller + activityLayer + screen + oscLayout + topRegion;
    QVERIFY(!combined.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!combined.contains(QStringLiteral("PlaybackCommandBus")));
    QVERIFY(!combined.contains(QStringLiteral("libmpv")));
    QVERIFY(!combined.contains(QStringLiteral("mpv_")));
}

} // namespace player::presentation::qml

QTEST_MAIN(player::presentation::qml::PlayerControlsVisibilityPolicyTest)
#include "controls_visibility_policy_test.moc"
