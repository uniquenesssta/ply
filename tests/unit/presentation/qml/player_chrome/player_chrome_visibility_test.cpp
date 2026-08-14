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

bool emitTimerTriggered(QObject* timer)
{
    return QMetaObject::invokeMethod(timer, "triggered", Qt::DirectConnection);
}

} // namespace

class PlayerChromeVisibilityTest final : public QObject
{
    Q_OBJECT

private slots:
    void inactivityTimerUsesWindowAndFullscreenDelays();
    void pauseScrubPopupAndErrorKeepOscVisible();
    void screenIntegratesSingleOscVisibilityOwner();
};

void PlayerChromeVisibilityTest::inactivityTimerUsesWindowAndFullscreenDelays()
{
    QQuickView view;
    QQuickItem* controller = createVisibilityController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));
    QVERIFY(view.isVisible());

    QObject* timer = findInactivityTimer(controller);
    QVERIFY(timer != nullptr);

    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(controller->property("visibilityLocked").toBool());
    QVERIFY(controller->setProperty("playing", true));

    // Regression guard: the playing-change handler must schedule from the
    // current primitive inputs, not from a derived binding whose evaluation
    // order relative to onPlayingChanged is unspecified by QML.
    QVERIFY(!controller->property("visibilityLocked").toBool());
    QVERIFY(timer->property("running").toBool());
    QCOMPARE(timer->property("interval").toInt(), kWindowHideDelayMs);
    QVERIFY(controller->property("chromeVisible").toBool());

    // CTest intentionally runs this target with QT_QPA_PLATFORM=offscreen.
    // Validate our scheduling contract deterministically: Timer is armed with
    // the canonical interval and its triggered handler drives the same hide
    // policy used in the product.
    QVERIFY(emitTimerTriggered(timer));
    QTRY_VERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(invokeActivity(controller, QStringLiteral("test-activity")));
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(timer->property("running").toBool());

    QVERIFY(controller->setProperty("fullScreen", true));
    QCOMPARE(timer->property("interval").toInt(), kFullscreenHideDelayMs);
    QVERIFY(timer->property("running").toBool());
    QVERIFY(controller->property("chromeVisible").toBool());

    QVERIFY(emitTimerTriggered(timer));
    QTRY_VERIFY(!controller->property("chromeVisible").toBool());
}

void PlayerChromeVisibilityTest::pauseScrubPopupAndErrorKeepOscVisible()
{
    QQuickView view;
    QQuickItem* controller = createVisibilityController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));
    QVERIFY(view.isVisible());

    QObject* timer = findInactivityTimer(controller);
    QVERIFY(timer != nullptr);

    QVERIFY(controller->setProperty("playing", true));
    QVERIFY(!controller->property("visibilityLocked").toBool());
    QVERIFY(timer->property("running").toBool());
    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("scrubbing", true));
    QVERIFY(controller->property("visibilityLocked").toBool());
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(!timer->property("running").toBool());
    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("scrubbing", false));
    QVERIFY(!controller->property("visibilityLocked").toBool());
    QVERIFY(timer->property("running").toBool());
    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("popupOpen", true));
    QVERIFY(controller->property("visibilityLocked").toBool());
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(!timer->property("running").toBool());
    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("popupOpen", false));
    QVERIFY(!controller->property("visibilityLocked").toBool());
    QVERIFY(timer->property("running").toBool());
    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("errorVisible", true));
    QVERIFY(controller->property("visibilityLocked").toBool());
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(!timer->property("running").toBool());
    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("errorVisible", false));
    QVERIFY(!controller->property("visibilityLocked").toBool());
    QVERIFY(timer->property("running").toBool());
    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("playing", false));
    QVERIFY(controller->property("visibilityLocked").toBool());
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(!timer->property("running").toBool());
    QVERIFY(invokeNoArg(controller, "hideIfEligible"));
    QVERIFY(controller->property("chromeVisible").toBool());
}

void PlayerChromeVisibilityTest::screenIntegratesSingleOscVisibilityOwner()
{
    const QString controller = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeVisibilityController.qml"));
    const QString activityLayer = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeActivityLayer.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString motion = readSource(QStringLiteral(
        "src/presentation/qml/theme/MotionTokens.qml"));

    QVERIFY(!controller.isEmpty());
    QVERIFY(!activityLayer.isEmpty());
    QVERIFY(!screen.isEmpty());
    QVERIFY(!motion.isEmpty());

    QVERIFY(controller.contains(QStringLiteral("Timer {")));
    QVERIFY(controller.contains(QStringLiteral("objectName: \"oscInactivityTimer\"")));
    QVERIFY(controller.contains(QStringLiteral("MotionTokens.oscHideDelay")));
    QVERIFY(controller.contains(QStringLiteral("MotionTokens.oscFullscreenHideDelay")));
    QVERIFY(controller.contains(QStringLiteral("onTriggered: root.hideIfEligible()")));
    QVERIFY(controller.contains(QStringLiteral("readonly property bool visibilityLocked: root.isVisibilityLocked()")));
    QVERIFY(controller.contains(QStringLiteral("function isVisibilityLocked()")));
    QVERIFY(controller.count(QStringLiteral("root.isVisibilityLocked()")) >= 4);
    QVERIFY(!controller.contains(QStringLiteral("if (root.visibilityLocked)")));
    QVERIFY(controller.contains(QStringLiteral("property bool scrubbing: false")));
    QVERIFY(controller.contains(QStringLiteral("property bool popupOpen: false")));
    QVERIFY(controller.contains(QStringLiteral("property bool errorVisible: false")));
    QVERIFY(controller.contains(QStringLiteral("console.info(")));

    QVERIFY(motion.contains(QStringLiteral("readonly property int oscHideDelay: 2200")));
    QVERIFY(motion.contains(QStringLiteral("readonly property int oscFullscreenHideDelay: 1600")));

    QVERIFY(activityLayer.contains(QStringLiteral("HoverHandler {")));
    QVERIFY(activityLayer.contains(QStringLiteral("onPointChanged:")));
    QVERIFY(!activityLayer.contains(QStringLiteral("MouseArea")));
    QVERIFY(!activityLayer.contains(QStringLiteral("Timer {")));

    QVERIFY(screen.contains(QStringLiteral("PlayerChromeVisibilityController {")));
    QVERIFY(screen.contains(QStringLiteral("PlayerChromeActivityLayer {")));
    QVERIFY(screen.contains(QStringLiteral("readonly property bool oscVisible")));
    QVERIFY(screen.contains(QStringLiteral("root.timelineViewModel.isScrubbing")));
    QVERIFY(screen.contains(QStringLiteral("root.timelineViewModel.seekPending")));
    QVERIFY(screen.contains(QStringLiteral("opacity: root.oscVisible")));
    QVERIFY(screen.contains(QStringLiteral("MotionTokens.oscShowDuration")));
    QVERIFY(screen.contains(QStringLiteral("MotionTokens.oscHideDuration")));
    QVERIFY(screen.contains(QStringLiteral("OpacityTokens.visible")));
    QVERIFY(screen.contains(QStringLiteral("OpacityTokens.hidden")));

    const QString combined = controller + activityLayer + screen;
    QVERIFY(!combined.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!combined.contains(QStringLiteral("PlaybackCommandBus")));
    QVERIFY(!combined.contains(QStringLiteral("libmpv")));
    QVERIFY(!combined.contains(QStringLiteral("mpv_")));
    QVERIFY(!combined.contains(QStringLiteral("CursorVisibilityController")));
    QVERIFY(!combined.contains(QStringLiteral("cursorShape")));
}

} // namespace player::presentation::qml

QTEST_MAIN(player::presentation::qml::PlayerChromeVisibilityTest)
#include "player_chrome_visibility_test.moc"