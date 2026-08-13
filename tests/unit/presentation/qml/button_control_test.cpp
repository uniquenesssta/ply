#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QStringList>
#include <QUrl>
#include <QtTest>

#include <memory>

namespace player::presentation::qml {
namespace {

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

} // namespace

class ButtonControlTest final : public QObject
{
    Q_OBJECT

private slots:
    void publicControlsResolveDesignGeometry();
    void pointerStateVisualContractsHold();
    void pointerAndKeyboardActivationWork();
    void disabledAndTooltipContractsHold();
    void reduceMotionFlowsIntoButtonStateTransitions();
};

void ButtonControlTest::publicControlsResolveDesignGeometry()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Item {
    width: 420
    height: 180

    IconButton {
        id: secondary
        objectName: "secondary"
        iconId: "previous"
    }

    IconButton {
        id: primary
        objectName: "primary"
        x: 60
        iconId: "play"
        emphasis: IconButton.Primary
    }

    TextButton {
        id: textButton
        objectName: "textButton"
        x: 120
        text: "Open"
    }

    ToggleButton {
        id: toggleButton
        objectName: "toggleButton"
        x: 220
        iconId: "subtitles"
        text: "Subtitles"
    }

    readonly property int playbackIconSize: LayoutTokens.playbackIcon
    readonly property real focusOpacity: OpacityTokens.focusRing
}
)QML"),
        QUrl(QStringLiteral("qrc:/ButtonControlGeometryContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    QObject* secondary = object->findChild<QObject*>(QStringLiteral("secondary"));
    QObject* primary = object->findChild<QObject*>(QStringLiteral("primary"));
    QObject* textButton = object->findChild<QObject*>(QStringLiteral("textButton"));
    QObject* toggleButton = object->findChild<QObject*>(QStringLiteral("toggleButton"));
    QVERIFY(secondary != nullptr);
    QVERIFY(primary != nullptr);
    QVERIFY(textButton != nullptr);
    QVERIFY(toggleButton != nullptr);

    QCOMPARE(secondary->property("width").toInt(), 32);
    QCOMPARE(secondary->property("height").toInt(), 32);
    QCOMPARE(secondary->property("visualIconSize").toInt(), 22);
    QCOMPARE(secondary->property("controlRadius").toInt(), 16);
    QCOMPARE(secondary->property("iconOpacity").toReal(), 0.72);

    QCOMPARE(primary->property("width").toInt(), 40);
    QCOMPARE(primary->property("height").toInt(), 40);
    QCOMPARE(primary->property("visualIconSize").toInt(), 24);
    QCOMPARE(primary->property("controlRadius").toInt(), 20);
    QCOMPARE(primary->property("surfaceAlpha").toReal(), 0.48);

    QCOMPARE(textButton->property("height").toInt(), 32);
    QVERIFY(textButton->property("width").toReal() >= 32.0);
    QCOMPARE(toggleButton->property("height").toInt(), 32);
    QVERIFY(toggleButton->property("width").toReal() >= 32.0);

    QCOMPARE(object->property("playbackIconSize").toInt(), 24);
    QCOMPARE(object->property("focusOpacity").toReal(), 0.82);
}

void ButtonControlTest::pointerStateVisualContractsHold()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Controls

Item {
    width: 260
    height: 140

    IconButton {
        objectName: "secondary"
        x: 20
        y: 20
        iconId: "previous"
    }

    IconButton {
        objectName: "primary"
        x: 80
        y: 20
        iconId: "play"
        emphasis: IconButton.Primary
        toolTipText: "Play"
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/ButtonControlPointerStateContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    auto* rootItem = qobject_cast<QQuickItem*>(object.get());
    auto* secondary = qobject_cast<QQuickItem*>(
        object->findChild<QObject*>(QStringLiteral("secondary")));
    auto* primary = qobject_cast<QQuickItem*>(
        object->findChild<QObject*>(QStringLiteral("primary")));
    QVERIFY(rootItem != nullptr);
    QVERIFY(secondary != nullptr);
    QVERIFY(primary != nullptr);

    QQuickWindow window;
    window.setGeometry(0, 0, 260, 140);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QTest::qWait(50);

    QTest::mouseMove(&window, QPoint(30, 30));
    QTRY_VERIFY_WITH_TIMEOUT(secondary->property("hovered").toBool(), 1000);
    QCOMPARE(secondary->property("interactionOpacity").toReal(), 1.0);
    QCOMPARE(secondary->property("iconOpacity").toReal(), 1.0);

    QTest::mousePress(
        &window,
        Qt::LeftButton,
        Qt::NoModifier,
        QPoint(30, 30));
    QTRY_VERIFY_WITH_TIMEOUT(secondary->property("pressed").toBool(), 1000);
    QCOMPARE(secondary->property("interactionOpacity").toReal(), 0.84);
    QTest::mouseRelease(
        &window,
        Qt::LeftButton,
        Qt::NoModifier,
        QPoint(30, 30));
    QTRY_VERIFY_WITH_TIMEOUT(!secondary->property("pressed").toBool(), 1000);

    QTest::mouseMove(&window, QPoint(90, 30));
    QTRY_VERIFY_WITH_TIMEOUT(primary->property("hovered").toBool(), 1000);
    QCOMPARE(primary->property("surfaceAlpha").toReal(), 0.52);

    QTest::mouseMove(&window, QPoint(220, 100));
    QTRY_VERIFY_WITH_TIMEOUT(!primary->property("hovered").toBool(), 1000);
    primary->forceActiveFocus(Qt::TabFocusReason);
    QTRY_VERIFY_WITH_TIMEOUT(primary->hasActiveFocus(), 1000);
    QCOMPARE(primary->property("surfaceAlpha").toReal(), 0.48);
    QCOMPARE(primary->property("toolTipVisible").toBool(), true);
}

void ButtonControlTest::pointerAndKeyboardActivationWork()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Controls

Item {
    width: 320
    height: 160

    TextButton {
        objectName: "textButton"
        x: 20
        y: 20
        text: "Open"
    }

    ToggleButton {
        objectName: "toggleButton"
        x: 20
        y: 80
        text: "Subtitles"
        toolTipText: "Subtitles"
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/ButtonControlInteractionContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    auto* rootItem = qobject_cast<QQuickItem*>(object.get());
    QVERIFY(rootItem != nullptr);

    auto* textButton = qobject_cast<QQuickItem*>(
        object->findChild<QObject*>(QStringLiteral("textButton")));
    auto* toggleButton = qobject_cast<QQuickItem*>(
        object->findChild<QObject*>(QStringLiteral("toggleButton")));
    QVERIFY(textButton != nullptr);
    QVERIFY(toggleButton != nullptr);

    QQuickWindow window;
    window.setGeometry(0, 0, 320, 160);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QTest::qWait(50);

    QSignalSpy textClicked(textButton, SIGNAL(clicked()));
    QVERIFY(textClicked.isValid());
    QTest::mouseClick(
        &window,
        Qt::LeftButton,
        Qt::NoModifier,
        QPoint(30, 30));
    QTRY_COMPARE_WITH_TIMEOUT(textClicked.count(), 1, 1000);

    QSignalSpy toggleClicked(toggleButton, SIGNAL(clicked()));
    QSignalSpy toggled(toggleButton, SIGNAL(toggled(bool)));
    QVERIFY(toggleClicked.isValid());
    QVERIFY(toggled.isValid());

    toggleButton->forceActiveFocus(Qt::TabFocusReason);
    QTRY_VERIFY_WITH_TIMEOUT(toggleButton->hasActiveFocus(), 1000);
    QTest::keyClick(&window, Qt::Key_Space);

    QTRY_COMPARE_WITH_TIMEOUT(toggleClicked.count(), 1, 1000);
    QTRY_COMPARE_WITH_TIMEOUT(toggled.count(), 1, 1000);
    QCOMPARE(toggleButton->property("checked").toBool(), true);
    QCOMPARE(toggled.at(0).at(0).toBool(), true);
}

void ButtonControlTest::disabledAndTooltipContractsHold()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Controls

Item {
    width: 260
    height: 120

    ToggleButton {
        objectName: "toggleButton"
        x: 20
        y: 20
        text: "Playlist"
        toolTipText: "Playlist"
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/ButtonControlAvailabilityContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    auto* rootItem = qobject_cast<QQuickItem*>(object.get());
    auto* toggleButton = qobject_cast<QQuickItem*>(
        object->findChild<QObject*>(QStringLiteral("toggleButton")));
    QVERIFY(rootItem != nullptr);
    QVERIFY(toggleButton != nullptr);

    QQuickWindow window;
    window.setGeometry(0, 0, 260, 120);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QTest::qWait(50);

    toggleButton->forceActiveFocus(Qt::TabFocusReason);
    QTRY_VERIFY_WITH_TIMEOUT(toggleButton->hasActiveFocus(), 1000);
    QTRY_VERIFY_WITH_TIMEOUT(
        toggleButton->property("toolTipVisible").toBool(),
        1000);

    QSignalSpy clicked(toggleButton, SIGNAL(clicked()));
    QSignalSpy toggled(toggleButton, SIGNAL(toggled(bool)));
    QVERIFY(clicked.isValid());
    QVERIFY(toggled.isValid());

    toggleButton->setEnabled(false);
    QTest::mouseClick(
        &window,
        Qt::LeftButton,
        Qt::NoModifier,
        QPoint(30, 30));
    QMetaObject::invokeMethod(toggleButton, "activate");
    QTest::qWait(20);

    QCOMPARE(clicked.count(), 0);
    QCOMPARE(toggled.count(), 0);
    QCOMPARE(toggleButton->property("checked").toBool(), false);
    QCOMPARE(toggleButton->property("toolTipVisible").toBool(), false);
}

void ButtonControlTest::reduceMotionFlowsIntoButtonStateTransitions()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Item {
    TextButton {
        id: button
        objectName: "button"
        text: "Open"
    }

    property bool reduceMotion: false
    onReduceMotionChanged: MotionTokens.reduceMotionEnabled = reduceMotion
    readonly property int transitionDuration: button.stateTransitionDuration
}
)QML"),
        QUrl(QStringLiteral("qrc:/ButtonControlMotionContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    QCOMPARE(object->property("transitionDuration").toInt(), 120);
    QVERIFY(object->setProperty("reduceMotion", true));
    QTRY_COMPARE_WITH_TIMEOUT(object->property("transitionDuration").toInt(), 0, 1000);
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::ButtonControlTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "button_control_test.moc"
