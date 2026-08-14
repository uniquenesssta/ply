#include <QFile>
#include <QMetaObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <QtTest>

#include <memory>

namespace player::presentation::qml {
namespace {

constexpr int kWindowPreHideProbeMs = 1700;
constexpr int kFullscreenPreHideProbeMs = 1200;
constexpr int kWindowHideCompletionTimeoutMs = 2000;
constexpr int kFullscreenHideCompletionTimeoutMs = 1800;

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

QString componentDiagnostics(const QQmlComponent& component)
{
    QStringList diagnostics;
    diagnostics.append(
        QStringLiteral("status=%1 progress=%2")
            .arg(static_cast<int>(component.status()))
            .arg(component.progress(), 0, 'f', 3));

    for (const QQmlError& error : component.errors()) {
        diagnostics.append(error.toString());
    }
    return diagnostics.join(QLatin1Char('\n'));
}

bool waitForComponentResolution(QQmlComponent& component)
{
    if (component.status() != QQmlComponent::Loading) {
        return true;
    }

    QSignalSpy statusSpy(&component, &QQmlComponent::statusChanged);
    return statusSpy.wait(5000);
}

std::unique_ptr<QObject> createVisibilityController(QQmlComponent& component)
{
    component.loadUrl(QUrl::fromLocalFile(sourcePath(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeVisibilityController.qml"))));

    if (!waitForComponentResolution(component) || !component.isReady()) {
        return {};
    }
    return std::unique_ptr<QObject>(component.create());
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

class PlayerChromeVisibilityTest final : public QObject
{
    Q_OBJECT

private slots:
    void playingInactivityUsesWindowAndFullscreenDelays();
    void pauseScrubPopupAndErrorKeepOscVisible();
    void screenIntegratesSingleOscVisibilityOwner();
};

void PlayerChromeVisibilityTest::playingInactivityUsesWindowAndFullscreenDelays()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    std::unique_ptr<QObject> controller = createVisibilityController(component);
    QVERIFY2(controller != nullptr, qPrintable(componentDiagnostics(component)));

    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(controller->setProperty("playing", true));

    QTest::qWait(kWindowPreHideProbeMs);
    QVERIFY(controller->property("chromeVisible").toBool());
    QTRY_VERIFY_WITH_TIMEOUT(
        !controller->property("chromeVisible").toBool(),
        kWindowHideCompletionTimeoutMs);

    QVERIFY(invokeActivity(controller.get(), QStringLiteral("test-activity")));
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(controller->setProperty("fullScreen", true));

    QTest::qWait(kFullscreenPreHideProbeMs);
    QVERIFY(controller->property("chromeVisible").toBool());
    QTRY_VERIFY_WITH_TIMEOUT(
        !controller->property("chromeVisible").toBool(),
        kFullscreenHideCompletionTimeoutMs);
}

void PlayerChromeVisibilityTest::pauseScrubPopupAndErrorKeepOscVisible()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    std::unique_ptr<QObject> controller = createVisibilityController(component);
    QVERIFY2(controller != nullptr, qPrintable(componentDiagnostics(component)));

    QVERIFY(controller->setProperty("playing", true));
    QVERIFY(invokeNoArg(controller.get(), "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("scrubbing", true));
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(invokeNoArg(controller.get(), "hideIfEligible"));
    QVERIFY(controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("scrubbing", false));
    QVERIFY(invokeNoArg(controller.get(), "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("popupOpen", true));
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(invokeNoArg(controller.get(), "hideIfEligible"));
    QVERIFY(controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("popupOpen", false));
    QVERIFY(invokeNoArg(controller.get(), "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("errorVisible", true));
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(invokeNoArg(controller.get(), "hideIfEligible"));
    QVERIFY(controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("errorVisible", false));
    QVERIFY(invokeNoArg(controller.get(), "hideIfEligible"));
    QVERIFY(!controller->property("chromeVisible").toBool());

    QVERIFY(controller->setProperty("playing", false));
    QVERIFY(controller->property("chromeVisible").toBool());
    QVERIFY(invokeNoArg(controller.get(), "hideIfEligible"));
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
    QVERIFY(controller.contains(QStringLiteral("MotionTokens.oscHideDelay")));
    QVERIFY(controller.contains(QStringLiteral("MotionTokens.oscFullscreenHideDelay")));
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
