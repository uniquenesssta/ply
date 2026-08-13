#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QStringList>
#include <QUrl>
#include <QWheelEvent>
#include <QtTest>

#include <cmath>
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

bool fuzzyEqual(double left, double right)
{
    return std::abs(left - right) <= 0.000001;
}

} // namespace

class SliderControlTest final : public QObject
{
    Q_OBJECT

private slots:
    void publicSliderResolvesDesignGeometry();
    void pointerDragPublishesNormalizedInteraction();
    void keyboardAndWheelAdjustNormalizedValue();
    void disabledAndClampContractsHold();
    void reduceMotionFlowsIntoSliderTransitions();
};

void SliderControlTest::publicSliderResolvesDesignGeometry()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Item {
    width: 320
    height: 120

    Slider {
        objectName: "slider"
        value: 0.6
    }

    readonly property real focusRingWidth: LayoutTokens.sliderFocusRingWidth
    readonly property real focusRingOpacity: OpacityTokens.focusRing
}
)QML"),
        QUrl(QStringLiteral("qrc:/SliderControlGeometryContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    QObject* slider = object->findChild<QObject*>(QStringLiteral("slider"));
    QObject* hitTarget = object->findChild<QObject*>(QStringLiteral("sliderHitTarget"));
    QObject* track = object->findChild<QObject*>(QStringLiteral("sliderTrack"));
    QObject* progress = object->findChild<QObject*>(QStringLiteral("sliderProgress"));
    QObject* thumb = object->findChild<QObject*>(QStringLiteral("sliderThumb"));
    QObject* valueLabel = object->findChild<QObject*>(QStringLiteral("sliderValue"));
    QVERIFY(slider != nullptr);
    QVERIFY(hitTarget != nullptr);
    QVERIFY(track != nullptr);
    QVERIFY(progress != nullptr);
    QVERIFY(thumb != nullptr);
    QVERIFY(valueLabel != nullptr);

    QCOMPARE(slider->property("width").toInt(), 180);
    QCOMPARE(slider->property("height").toInt(), 32);
    QCOMPARE(slider->property("hitTargetWidth").toInt(), 132);
    QCOMPARE(slider->property("trackWidth").toInt(), 126);
    QVERIFY(fuzzyEqual(slider->property("progressWidth").toDouble(), 75.6));
    QCOMPARE(slider->property("visualThumbSize").toInt(), 10);
    QCOMPARE(slider->property("valueText").toString(), QStringLiteral("60%"));

    QCOMPARE(hitTarget->property("height").toInt(), 16);
    QCOMPARE(track->property("height").toInt(), 3);
    QVERIFY(fuzzyEqual(progress->property("width").toDouble(), 75.6));
    QCOMPARE(thumb->property("width").toInt(), 10);
    QCOMPARE(valueLabel->property("width").toInt(), 36);

    QVERIFY(fuzzyEqual(object->property("focusRingWidth").toDouble(), 1.5));
    QVERIFY(fuzzyEqual(object->property("focusRingOpacity").toDouble(), 0.82));
}

void SliderControlTest::pointerDragPublishesNormalizedInteraction()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Controls

Item {
    width: 260
    height: 100

    Slider {
        objectName: "slider"
        x: 20
        y: 20
        value: 0.5
        stepSize: 0.01
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/SliderControlPointerContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    auto* rootItem = qobject_cast<QQuickItem*>(object.get());
    auto* slider = qobject_cast<QQuickItem*>(
        object->findChild<QObject*>(QStringLiteral("slider")));
    QVERIFY(rootItem != nullptr);
    QVERIFY(slider != nullptr);

    QQuickWindow window;
    window.setGeometry(0, 0, 260, 100);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QTest::qWait(50);

    QSignalSpy started(slider, SIGNAL(interactionStarted()));
    QSignalSpy edited(slider, SIGNAL(valueEdited(double)));
    QSignalSpy finished(slider, SIGNAL(interactionFinished(double)));
    QVERIFY(started.isValid());
    QVERIFY(edited.isValid());
    QVERIFY(finished.isValid());

    QTest::mousePress(
        &window,
        Qt::LeftButton,
        Qt::NoModifier,
        QPoint(55, 36));
    QTRY_VERIFY_WITH_TIMEOUT(slider->property("pressed").toBool(), 1000);
    QVERIFY(fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.25));
    QCOMPARE(slider->property("visualThumbSize").toInt(), 14);

    QTest::mouseMove(&window, QPoint(118, 36));
    QTRY_VERIFY_WITH_TIMEOUT(
        fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.75),
        1000);

    QTest::mouseRelease(
        &window,
        Qt::LeftButton,
        Qt::NoModifier,
        QPoint(118, 36));
    QTRY_VERIFY_WITH_TIMEOUT(!slider->property("pressed").toBool(), 1000);
    QVERIFY(fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.75));

    QCOMPARE(started.count(), 1);
    QVERIFY(edited.count() >= 2);
    QCOMPARE(finished.count(), 1);
    QVERIFY(fuzzyEqual(finished.at(0).at(0).toDouble(), 0.75));
}

void SliderControlTest::keyboardAndWheelAdjustNormalizedValue()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Controls

Item {
    width: 260
    height: 100

    Slider {
        objectName: "slider"
        x: 20
        y: 20
        value: 0.5
        stepSize: 0.1
        wheelStep: 0.2
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/SliderControlKeyboardWheelContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    auto* rootItem = qobject_cast<QQuickItem*>(object.get());
    auto* slider = qobject_cast<QQuickItem*>(
        object->findChild<QObject*>(QStringLiteral("slider")));
    QVERIFY(rootItem != nullptr);
    QVERIFY(slider != nullptr);

    QQuickWindow window;
    window.setGeometry(0, 0, 260, 100);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QTest::qWait(50);

    slider->forceActiveFocus(Qt::TabFocusReason);
    QTRY_VERIFY_WITH_TIMEOUT(slider->hasActiveFocus(), 1000);

    QTest::keyClick(&window, Qt::Key_Right);
    QTRY_VERIFY_WITH_TIMEOUT(
        fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.6),
        1000);

    QTest::keyClick(&window, Qt::Key_Left);
    QTRY_VERIFY_WITH_TIMEOUT(
        fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.5),
        1000);

    QWheelEvent wheelUp(
        QPointF(50, 36),
        QPointF(50, 36),
        QPoint(),
        QPoint(0, 120),
        Qt::NoButton,
        Qt::NoModifier,
        Qt::NoScrollPhase,
        false);
    QCoreApplication::sendEvent(&window, &wheelUp);
    QTRY_VERIFY_WITH_TIMEOUT(
        fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.7),
        1000);

    QWheelEvent wheelDown(
        QPointF(50, 36),
        QPointF(50, 36),
        QPoint(),
        QPoint(0, -120),
        Qt::NoButton,
        Qt::NoModifier,
        Qt::NoScrollPhase,
        false);
    QCoreApplication::sendEvent(&window, &wheelDown);
    QTRY_VERIFY_WITH_TIMEOUT(
        fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.5),
        1000);
}

void SliderControlTest::disabledAndClampContractsHold()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Controls

Item {
    width: 320
    height: 120

    Slider {
        objectName: "slider"
        x: 20
        y: 20
        width: 240
        value: 0.5
        stepSize: 0.1
        showValue: false
        enabled: false
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/SliderControlDisabledContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    auto* rootItem = qobject_cast<QQuickItem*>(object.get());
    auto* slider = qobject_cast<QQuickItem*>(
        object->findChild<QObject*>(QStringLiteral("slider")));
    QVERIFY(rootItem != nullptr);
    QVERIFY(slider != nullptr);

    QCOMPARE(slider->property("hitTargetWidth").toInt(), 240);
    QCOMPARE(slider->property("trackWidth").toInt(), 234);
    QVERIFY(fuzzyEqual(slider->property("interactionOpacity").toDouble(), 0.38));

    slider->setProperty("value", 1.4);
    QVERIFY(fuzzyEqual(slider->property("normalizedValue").toDouble(), 1.0));
    QVERIFY(fuzzyEqual(slider->property("value").toDouble(), 1.0));

    slider->setProperty("value", -0.4);
    QVERIFY(fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.0));
    QVERIFY(fuzzyEqual(slider->property("value").toDouble(), 0.0));

    slider->setProperty("value", 0.5);

    QQuickWindow window;
    window.setGeometry(0, 0, 320, 120);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QTest::qWait(50);

    QSignalSpy edited(slider, SIGNAL(valueEdited(double)));
    QVERIFY(edited.isValid());

    QTest::mouseClick(
        &window,
        Qt::LeftButton,
        Qt::NoModifier,
        QPoint(200, 36));
    QTest::keyClick(&window, Qt::Key_Right);

    QCOMPARE(edited.count(), 0);
    QVERIFY(fuzzyEqual(slider->property("normalizedValue").toDouble(), 0.5));
}

void SliderControlTest::reduceMotionFlowsIntoSliderTransitions()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Item {
    property bool reduceMotion: false
    onReduceMotionChanged: MotionTokens.reduceMotionEnabled = reduceMotion

    Slider {
        objectName: "slider"
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/SliderControlReduceMotionContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    QObject* slider = object->findChild<QObject*>(QStringLiteral("slider"));
    QVERIFY(slider != nullptr);

    QVERIFY(slider->property("stateTransitionDuration").toInt() > 0);
    object->setProperty("reduceMotion", true);
    QTRY_COMPARE_WITH_TIMEOUT(slider->property("stateTransitionDuration").toInt(), 0, 1000);

    object->setProperty("reduceMotion", false);
    QTRY_VERIFY_WITH_TIMEOUT(slider->property("stateTransitionDuration").toInt() > 0, 1000);
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::SliderControlTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "slider_control_test.moc"
