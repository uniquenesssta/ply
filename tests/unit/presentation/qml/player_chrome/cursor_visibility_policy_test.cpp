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

void prepareEligibleHiddenState(QQuickItem* controller)
{
    QVERIFY(controller->setProperty("playing", true));
    QVERIFY(controller->setProperty("windowActive", true));
    QVERIFY(controller->setProperty("pointerInside", true));
    QVERIFY(notifyActivity(controller, QStringLiteral("test-setup")));
    QVERIFY(controller->setProperty("oscVisible", false));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());
}

} // namespace

class CursorVisibilityPolicyTest final : public QObject
{
    Q_OBJECT

private slots:
    void interactionLocksForceCursorVisible();
    void pointerLeaveRequiresActivityBeforeRehide();
    void focusLossAndActivationRequireActivityBeforeRehide();
    void rapidActivationCyclesKeepSingleTimerOwner();
    void policyIntegrationStaysWindowLocalAndTimerFree();
};

void CursorVisibilityPolicyTest::interactionLocksForceCursorVisible()
{
    QQuickView view;
    QQuickItem* controller = createCursorController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));
    prepareEligibleHiddenState(controller);

    const QStringList lockProperties{
        QStringLiteral("scrubbing"),
        QStringLiteral("dragActive"),
        QStringLiteral("popupOpen"),
        QStringLiteral("menuOpen"),
        QStringLiteral("drawerOpen"),
        QStringLiteral("modalActive"),
        QStringLiteral("errorVisible"),
        QStringLiteral("cursorHideSuppressed"),
    };

    for (const QString& propertyName : lockProperties) {
        QVERIFY2(controller->setProperty(propertyName.toUtf8().constData(), true),
                 qPrintable(propertyName));
        QTRY_VERIFY2(!controller->property("cursorHidden").toBool(),
                     qPrintable(propertyName));
        QVERIFY2(controller->setProperty(propertyName.toUtf8().constData(), false),
                 qPrintable(propertyName));
        QTRY_VERIFY2(controller->property("cursorHidden").toBool(),
                     qPrintable(propertyName));
    }

    QVERIFY(controller->setProperty("oscVisible", true));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());
    QVERIFY(controller->setProperty("oscVisible", false));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("playing", false));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());
}

void CursorVisibilityPolicyTest::pointerLeaveRequiresActivityBeforeRehide()
{
    QQuickView view;
    QQuickItem* controller = createCursorController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));
    prepareEligibleHiddenState(controller);

    QVERIFY(controller->setProperty("pointerInside", false));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("pointerInside", true));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());

    QVERIFY(notifyActivity(controller, QStringLiteral("pointer-enter")));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());
}

void CursorVisibilityPolicyTest::focusLossAndActivationRequireActivityBeforeRehide()
{
    QQuickView view;
    QQuickItem* controller = createCursorController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));
    prepareEligibleHiddenState(controller);

    QVERIFY(controller->setProperty("windowActive", false));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());

    QVERIFY(controller->setProperty("windowActive", true));
    QTRY_VERIFY(!controller->property("cursorHidden").toBool());

    QVERIFY(notifyActivity(controller, QStringLiteral("window-activity")));
    QTRY_VERIFY(controller->property("cursorHidden").toBool());
}

void CursorVisibilityPolicyTest::rapidActivationCyclesKeepSingleTimerOwner()
{
    QQuickView view;
    QQuickItem* controller = createCursorController(view);
    QVERIFY2(controller != nullptr, qPrintable(viewDiagnostics(view)));
    prepareEligibleHiddenState(controller);

    for (int index = 0; index < 10; ++index) {
        QVERIFY(controller->setProperty("windowActive", false));
        QTRY_VERIFY(!controller->property("cursorHidden").toBool());
        QVERIFY(controller->setProperty("windowActive", true));
        QTRY_VERIFY(!controller->property("cursorHidden").toBool());
        QVERIFY(notifyActivity(controller, QStringLiteral("activation-cycle")));
        QTRY_VERIFY(controller->property("cursorHidden").toBool());
    }

    const QString chromeController = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeVisibilityController.qml"));
    const QString cursorController = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerCursorVisibilityController.qml"));
    const QString activityLayer = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeActivityLayer.qml"));

    QCOMPARE(chromeController.count(QStringLiteral("Timer {")), 1);
    QVERIFY(!cursorController.contains(QStringLiteral("Timer {")));
    QVERIFY(!activityLayer.contains(QStringLiteral("Timer {")));
}

void CursorVisibilityPolicyTest::policyIntegrationStaysWindowLocalAndTimerFree()
{
    const QString cursorController = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerCursorVisibilityController.qml"));
    const QString activityLayer = readSource(QStringLiteral(
        "src/presentation/qml/features/player/chrome/PlayerChromeActivityLayer.qml"));
    const QString volumeControls = readSource(QStringLiteral(
        "src/presentation/qml/features/player/volume/VolumeControls.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString mainWindow = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));

    QVERIFY(!cursorController.isEmpty());
    QVERIFY(!activityLayer.isEmpty());
    QVERIFY(!volumeControls.isEmpty());
    QVERIFY(!screen.isEmpty());
    QVERIFY(!mainWindow.isEmpty());

    QVERIFY(cursorController.contains(QStringLiteral("property bool dragActive: false")));
    QVERIFY(cursorController.contains(QStringLiteral("property bool menuOpen: false")));
    QVERIFY(cursorController.contains(QStringLiteral("property bool drawerOpen: false")));
    QVERIFY(cursorController.contains(QStringLiteral("property bool modalActive: false")));
    QVERIFY(cursorController.contains(QStringLiteral("property bool windowActive: true")));
    QVERIFY(cursorController.contains(QStringLiteral("property bool pointerInside: false")));
    QVERIFY(cursorController.contains(QStringLiteral("restoreVisibleUntilActivity")));
    QVERIFY(cursorController.contains(QStringLiteral("onWindowActiveChanged:")));
    QVERIFY(cursorController.contains(QStringLiteral("onPointerInsideChanged:")));
    QVERIFY(cursorController.contains(QStringLiteral("R6-15 cursor visibility")));

    QVERIFY(activityLayer.contains(QStringLiteral(
        "readonly property bool pointerInside: pointerHover.hovered")));
    QVERIFY(activityLayer.contains(QStringLiteral(
        "cursorShape: root.cursorHidden ? Qt.BlankCursor : undefined")));
    QVERIFY(volumeControls.contains(QStringLiteral(
        "readonly property bool interactionActive: volumeSlider.pressed")));

    QVERIFY(screen.contains(QStringLiteral(
        "readonly property bool controlsDragActive: root.dragActive")));
    QVERIFY(screen.contains(QStringLiteral("|| volumeControls.interactionActive")));
    QVERIFY(screen.contains(QStringLiteral("dragActive: root.controlsDragActive")));
    QVERIFY(screen.contains(QStringLiteral("id: volumeControls")));
    QVERIFY(screen.contains(QStringLiteral("menuOpen: root.menuOpen")));
    QVERIFY(screen.contains(QStringLiteral("property bool playlistDrawerOpen: false")));
    QVERIFY(screen.contains(QStringLiteral("readonly property bool anyDrawerOpen: root.drawerOpen")));
    QVERIFY(screen.contains(QStringLiteral("|| root.playlistDrawerOpen")));
    QVERIFY(screen.contains(QStringLiteral("|| root.chapterDrawerOpen")));
    QCOMPARE(screen.count(QStringLiteral("drawerOpen: root.anyDrawerOpen")), 2);
    QVERIFY(screen.contains(QStringLiteral("modalActive: root.modalActive")));
    QVERIFY(screen.contains(QStringLiteral("windowActive: root.windowActive")));
    QVERIFY(screen.contains(QStringLiteral("pointerInside: root.pointerInsideWindow")));
    QVERIFY(screen.contains(QStringLiteral(
        "readonly property bool pointerInsideWindow: chromeActivityLayer.pointerInside")));
    QVERIFY(screen.contains(QStringLiteral(
        "chromeVisibilityController.notifyActivity(reason)")));
    QVERIFY(screen.contains(QStringLiteral(
        "cursorVisibilityController.notifyActivity(reason)")));
    QVERIFY(mainWindow.contains(QStringLiteral("windowActive: window.active")));

    const QString combined = cursorController + activityLayer + volumeControls + screen;
    QVERIFY(!combined.contains(QStringLiteral("setOverrideCursor")));
    QVERIFY(!combined.contains(QStringLiteral("restoreOverrideCursor")));
    QVERIFY(!combined.contains(QStringLiteral("QGuiApplication")));
    QVERIFY(!combined.contains(QStringLiteral("Component.onDestruction")));
    QVERIFY(!cursorController.contains(QStringLiteral("Timer {")));
    QVERIFY(!activityLayer.contains(QStringLiteral("Timer {")));
    QVERIFY(!volumeControls.contains(QStringLiteral("Timer {")));
    QVERIFY(!combined.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!combined.contains(QStringLiteral("PlaybackCommandBus")));
    QVERIFY(!combined.contains(QStringLiteral("libmpv")));
    QVERIFY(!combined.contains(QStringLiteral("mpv_")));
}

} // namespace player::presentation::qml

QTEST_MAIN(player::presentation::qml::CursorVisibilityPolicyTest)
#include "cursor_visibility_policy_test.moc"
