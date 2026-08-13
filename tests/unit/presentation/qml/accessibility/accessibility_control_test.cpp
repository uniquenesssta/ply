#include <QFile>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QStringList>
#include <QUrl>
#include <QtTest>

#include <memory>

namespace player::presentation::qml {
namespace {

QString diagnostics(const QQmlComponent& component)
{
    QStringList result;
    for (const QQmlError& error : component.errors()) {
        result.append(error.toString());
    }
    return result.join(QLatin1Char('\n'));
}

bool resolve(QQmlComponent& component)
{
    if (component.status() != QQmlComponent::Loading) {
        return true;
    }

    QSignalSpy spy(&component, &QQmlComponent::statusChanged);
    return spy.wait(5000);
}

QString readSource(const QString& relativePath)
{
    QFile file(QStringLiteral(PLAYER_SOURCE_DIR "/") + relativePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

qreal focusBorderWidth(QObject* item)
{
    return QQmlProperty(item, QStringLiteral("border.width")).read().toReal();
}

} // namespace

class AccessibilityControlTest final : public QObject
{
    Q_OBJECT

private slots:
    void controlsDeclareAccessibleMetadata();
    void semanticNamesResolve();
    void tabNavigationKeepsFocusVisible();
};

void AccessibilityControlTest::controlsDeclareAccessibleMetadata()
{
    const QString buttonBase = readSource(
        QStringLiteral("src/presentation/qml/controls/buttons/ButtonBase.qml"));
    const QString slider = readSource(
        QStringLiteral("src/presentation/qml/controls/sliders/Slider.qml"));

    QVERIFY(!buttonBase.isEmpty());
    QVERIFY(!slider.isEmpty());

    QVERIFY(buttonBase.contains(QStringLiteral("Accessible.role: Accessible.Button")));
    QVERIFY(buttonBase.contains(QStringLiteral("Accessible.name: root.accessibleName")));
    QVERIFY(buttonBase.contains(QStringLiteral("Accessible.focusable: root.enabled")));
    QVERIFY(buttonBase.contains(QStringLiteral("Accessible.checkable: root.toggleOnActivate")));
    QVERIFY(buttonBase.contains(QStringLiteral("Accessible.checked: root.checked")));
    QVERIFY(buttonBase.contains(QStringLiteral("Accessible.onPressAction: root.activate()")));

    QVERIFY(slider.contains(QStringLiteral("Accessible.role: Accessible.Slider")));
    QVERIFY(slider.contains(QStringLiteral("Accessible.name: root.accessibleName")));
    QVERIFY(slider.contains(QStringLiteral("Accessible.focusable: root.enabled")));
}

void AccessibilityControlTest::semanticNamesResolve()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Controls

Item {
    TextButton {
        objectName: "textButton"
        text: "Open"
    }
    IconButton {
        objectName: "iconButton"
        x: 60
        iconId: "play"
        toolTipText: "Play"
    }
    ToggleButton {
        objectName: "toggleButton"
        x: 120
        text: "Subtitles"
    }
    Slider {
        objectName: "slider"
        x: 220
        accessibleName: "Volume"
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/AccessibilityNames.qml")));

    QVERIFY2(resolve(component), qPrintable(diagnostics(component)));
    QVERIFY2(component.isReady(), qPrintable(diagnostics(component)));

    std::unique_ptr<QObject> root(component.create());
    QVERIFY2(root != nullptr, qPrintable(diagnostics(component)));

    QObject* textButton = root->findChild<QObject*>(QStringLiteral("textButton"));
    QObject* iconButton = root->findChild<QObject*>(QStringLiteral("iconButton"));
    QObject* toggleButton = root->findChild<QObject*>(QStringLiteral("toggleButton"));
    QObject* slider = root->findChild<QObject*>(QStringLiteral("slider"));
    QVERIFY(textButton && iconButton && toggleButton && slider);

    QCOMPARE(textButton->property("accessibleName").toString(), QStringLiteral("Open"));
    QCOMPARE(iconButton->property("accessibleName").toString(), QStringLiteral("Play"));
    QCOMPARE(toggleButton->property("accessibleName").toString(), QStringLiteral("Subtitles"));
    QCOMPARE(slider->property("accessibleName").toString(), QStringLiteral("Volume"));
}

void AccessibilityControlTest::tabNavigationKeepsFocusVisible()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Controls

Item {
    width: 420
    height: 120

    TextButton {
        objectName: "first"
        x: 20
        y: 20
        text: "Open"
    }
    IconButton {
        objectName: "second"
        x: 90
        y: 20
        iconId: "play"
        toolTipText: "Play"
    }
    ToggleButton {
        objectName: "disabled"
        x: 150
        y: 20
        text: "Disabled"
        enabled: false
    }
    Slider {
        objectName: "third"
        x: 220
        y: 20
        width: 180
        accessibleName: "Volume"
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/AccessibilityTab.qml")));

    QVERIFY2(resolve(component), qPrintable(diagnostics(component)));
    QVERIFY2(component.isReady(), qPrintable(diagnostics(component)));

    std::unique_ptr<QObject> root(component.create());
    QVERIFY2(root != nullptr, qPrintable(diagnostics(component)));

    auto* rootItem = qobject_cast<QQuickItem*>(root.get());
    auto* first = qobject_cast<QQuickItem*>(
        root->findChild<QObject*>(QStringLiteral("first")));
    auto* second = qobject_cast<QQuickItem*>(
        root->findChild<QObject*>(QStringLiteral("second")));
    auto* disabled = qobject_cast<QQuickItem*>(
        root->findChild<QObject*>(QStringLiteral("disabled")));
    auto* third = qobject_cast<QQuickItem*>(
        root->findChild<QObject*>(QStringLiteral("third")));
    QVERIFY(rootItem && first && second && disabled && third);

    QQuickWindow window;
    window.setGeometry(0, 0, 440, 140);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QTest::qWait(50);

    first->forceActiveFocus(Qt::TabFocusReason);
    QTRY_VERIFY_WITH_TIMEOUT(first->hasActiveFocus(), 1000);
    QObject* firstRing = first->findChild<QObject*>(QStringLiteral("buttonFocusRing"));
    QVERIFY(firstRing != nullptr);
    QVERIFY(focusBorderWidth(firstRing) > 0.0);

    QTest::keyClick(&window, Qt::Key_Tab);
    QTRY_VERIFY_WITH_TIMEOUT(second->hasActiveFocus(), 1000);
    QObject* secondRing = second->findChild<QObject*>(QStringLiteral("buttonFocusRing"));
    QVERIFY(secondRing != nullptr);
    QVERIFY(focusBorderWidth(secondRing) > 0.0);

    QTest::keyClick(&window, Qt::Key_Tab);
    QTRY_VERIFY_WITH_TIMEOUT(third->hasActiveFocus(), 1000);
    QVERIFY(!disabled->hasActiveFocus());

    QObject* sliderRing = third->findChild<QObject*>(QStringLiteral("sliderFocusRing"));
    QVERIFY(sliderRing != nullptr);
    QVERIFY(focusBorderWidth(sliderRing) > 0.0);
    QVERIFY(sliderRing->property("opacity").toReal() > 0.0);
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::AccessibilityControlTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "accessibility_control_test.moc"
