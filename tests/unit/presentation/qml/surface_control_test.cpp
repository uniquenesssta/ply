#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QStringList>
#include <QUrl>
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

QStringList surfaceImportViolations()
{
    QStringList violations;
    const QString surfaceRoot =
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/surfaces");
    QDirIterator iterator(
        surfaceRoot,
        {QStringLiteral("*.qml")},
        QDir::Files,
        QDirIterator::Subdirectories);
    const QRegularExpression moduleImport(
        QStringLiteral(R"(^\s*import\s+(Player\.Presentation\.[A-Za-z0-9_.]+))"));

    while (iterator.hasNext()) {
        const QString filePath = iterator.next();
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            violations.append(QStringLiteral("cannot read %1").arg(filePath));
            continue;
        }

        const QStringList lines =
            QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
        for (qsizetype index = 0; index < lines.size(); ++index) {
            const QRegularExpressionMatch match = moduleImport.match(lines.at(index));
            if (!match.hasMatch()) {
                continue;
            }

            const QString module = match.captured(1);
            if (module != QStringLiteral("Player.Presentation.Theme")) {
                violations.append(
                    QStringLiteral("%1:%2 imports forbidden design module %3")
                        .arg(filePath)
                        .arg(index + 1)
                        .arg(module));
            }
        }

        file.seek(0);
        const QString content = QString::fromUtf8(file.readAll());
        if (content.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive)
            || content.contains(QStringLiteral("PlaybackSession"), Qt::CaseSensitive)) {
            violations.append(
                QStringLiteral("%1 contains playback/backend vocabulary").arg(filePath));
        }
    }

    return violations;
}

QObject* requireChild(QObject* root, const char* name)
{
    QObject* child = root->findChild<QObject*>(QString::fromLatin1(name));
    if (!child) {
        qWarning() << "missing child" << name;
    }
    return child;
}

} // namespace

class SurfaceControlTest final : public QObject
{
    Q_OBJECT

private slots:
    void publicSurfaceRolesResolveDesignContracts();
    void surfaceSlotsHostReplaceableContent();
    void overlayStackUsesSemanticZOrder();
    void surfaceModuleRemainsPresentationOnly();
};

void SurfaceControlTest::publicSurfaceRolesResolveDesignContracts()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Surfaces

Item {
    Panel { objectName: "panel"; width: 240; height: 140 }
    OscSurface { objectName: "osc"; width: 880; height: 124 }
    OscSurface { objectName: "oscCompact"; compact: true; width: 600; height: 106 }
    Drawer { objectName: "drawer"; width: 240; height: 140 }
    Popover { objectName: "popover"; width: 200; height: 100 }
    Hud { objectName: "hud"; width: 180; height: 80 }
}
)QML"),
        QUrl(QStringLiteral("qrc:/SurfaceRoleContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    QObject* panel = requireChild(object.get(), "panel");
    QObject* osc = requireChild(object.get(), "osc");
    QObject* oscCompact = requireChild(object.get(), "oscCompact");
    QObject* drawer = requireChild(object.get(), "drawer");
    QObject* popover = requireChild(object.get(), "popover");
    QObject* hud = requireChild(object.get(), "hud");
    QVERIFY(panel != nullptr);
    QVERIFY(osc != nullptr);
    QVERIFY(oscCompact != nullptr);
    QVERIFY(drawer != nullptr);
    QVERIFY(popover != nullptr);
    QVERIFY(hud != nullptr);

    QCOMPARE(panel->property("cornerRadius").toInt(), 32);
    QVERIFY(fuzzyEqual(panel->property("fillAlpha").toDouble(), 0.38));
    QCOMPARE(panel->property("backdropBlurRadius").toInt(), 42);
    QCOMPARE(panel->property("shadowRadius").toInt(), 42);
    QVERIFY(fuzzyEqual(panel->property("shadowYOffset").toDouble(), 12.0));
    QVERIFY(fuzzyEqual(panel->property("shadowAlpha").toDouble(), 0.12));
    QCOMPARE(panel->property("borderWidth").toInt(), 1);
    QVERIFY(fuzzyEqual(panel->property("borderAlpha").toDouble(), 0.48));
    QCOMPARE(panel->property("contentPadding").toInt(), 22);
    QVERIFY(fuzzyEqual(panel->property("z").toDouble(), 35.0));

    QCOMPARE(osc->property("cornerRadius").toInt(), 34);
    QVERIFY(fuzzyEqual(osc->property("fillAlpha").toDouble(), 0.34));
    QCOMPARE(osc->property("backdropBlurRadius").toInt(), 36);
    QCOMPARE(osc->property("shadowRadius").toInt(), 42);
    QVERIFY(fuzzyEqual(osc->property("shadowYOffset").toDouble(), 12.0));
    QVERIFY(fuzzyEqual(osc->property("shadowAlpha").toDouble(), 0.12));
    QCOMPARE(osc->property("borderWidth").toInt(), 1);
    QVERIFY(fuzzyEqual(osc->property("borderAlpha").toDouble(), 0.48));
    QCOMPARE(osc->property("contentPadding").toInt(), 0);
    QCOMPARE(osc->property("implicitWidth").toInt(), 880);
    QCOMPARE(osc->property("implicitHeight").toInt(), 124);
    QVERIFY(fuzzyEqual(osc->property("z").toDouble(), 40.0));

    QCOMPARE(oscCompact->property("cornerRadius").toInt(), 32);
    QVERIFY(fuzzyEqual(oscCompact->property("fillAlpha").toDouble(), 0.32));
    QCOMPARE(oscCompact->property("backdropBlurRadius").toInt(), 38);
    QCOMPARE(oscCompact->property("contentPadding").toInt(), 0);
    QCOMPARE(oscCompact->property("implicitWidth").toInt(), 880);
    QCOMPARE(oscCompact->property("implicitHeight").toInt(), 106);
    QVERIFY(fuzzyEqual(oscCompact->property("z").toDouble(), 40.0));

    QCOMPARE(drawer->property("cornerRadius").toInt(), 32);
    QVERIFY(fuzzyEqual(drawer->property("fillAlpha").toDouble(), 0.38));
    QCOMPARE(drawer->property("backdropBlurRadius").toInt(), 42);
    QCOMPARE(drawer->property("shadowRadius").toInt(), 42);
    QVERIFY(fuzzyEqual(drawer->property("z").toDouble(), 50.0));

    QCOMPARE(popover->property("cornerRadius").toInt(), 20);
    QVERIFY(fuzzyEqual(popover->property("fillAlpha").toDouble(), 0.52));
    QCOMPARE(popover->property("backdropBlurRadius").toInt(), 18);
    QCOMPARE(popover->property("shadowRadius").toInt(), 16);
    QVERIFY(fuzzyEqual(popover->property("shadowYOffset").toDouble(), 5.0));
    QVERIFY(fuzzyEqual(popover->property("shadowAlpha").toDouble(), 0.08));
    QCOMPARE(popover->property("contentPadding").toInt(), 18);
    QVERIFY(fuzzyEqual(popover->property("z").toDouble(), 60.0));

    QCOMPARE(hud->property("cornerRadius").toInt(), 24);
    QVERIFY(fuzzyEqual(hud->property("fillAlpha").toDouble(), 0.48));
    QCOMPARE(hud->property("backdropBlurRadius").toInt(), 18);
    QCOMPARE(hud->property("shadowRadius").toInt(), 16);
    QVERIFY(fuzzyEqual(hud->property("shadowYOffset").toDouble(), 5.0));
    QVERIFY(fuzzyEqual(hud->property("shadowAlpha").toDouble(), 0.08));
    QCOMPARE(hud->property("contentPadding").toInt(), 18);
    QVERIFY(fuzzyEqual(hud->property("z").toDouble(), 70.0));
}

void SurfaceControlTest::surfaceSlotsHostReplaceableContent()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Surfaces

Item {
    Panel {
        objectName: "panel"
        width: 200
        height: 120

        Rectangle {
            objectName: "panelSlot"
            anchors.fill: parent
        }
    }

    OscSurface {
        objectName: "osc"
        width: 880
        height: 124

        Item {
            objectName: "oscSlot"
            anchors.fill: parent
        }
    }

    Popover {
        objectName: "popover"
        width: 180
        height: 100

        Item {
            objectName: "popoverSlot"
            anchors.fill: parent
        }
    }
}
)QML"),
        QUrl(QStringLiteral("qrc:/SurfaceSlotContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    auto* panelSlot = qobject_cast<QQuickItem*>(requireChild(object.get(), "panelSlot"));
    auto* oscSlot = qobject_cast<QQuickItem*>(requireChild(object.get(), "oscSlot"));
    auto* popoverSlot = qobject_cast<QQuickItem*>(requireChild(object.get(), "popoverSlot"));
    QVERIFY(panelSlot != nullptr);
    QVERIFY(oscSlot != nullptr);
    QVERIFY(popoverSlot != nullptr);
    QVERIFY(panelSlot->parentItem() != nullptr);
    QVERIFY(oscSlot->parentItem() != nullptr);
    QVERIFY(popoverSlot->parentItem() != nullptr);

    QCOMPARE(panelSlot->parentItem()->objectName(), QStringLiteral("surfaceContent"));
    QVERIFY(fuzzyEqual(panelSlot->parentItem()->x(), 22.0));
    QVERIFY(fuzzyEqual(panelSlot->parentItem()->y(), 22.0));
    QVERIFY(fuzzyEqual(panelSlot->parentItem()->width(), 156.0));
    QVERIFY(fuzzyEqual(panelSlot->parentItem()->height(), 76.0));

    QCOMPARE(oscSlot->parentItem()->objectName(), QStringLiteral("surfaceContent"));
    QVERIFY(fuzzyEqual(oscSlot->parentItem()->x(), 0.0));
    QVERIFY(fuzzyEqual(oscSlot->parentItem()->y(), 0.0));
    QVERIFY(fuzzyEqual(oscSlot->parentItem()->width(), 880.0));
    QVERIFY(fuzzyEqual(oscSlot->parentItem()->height(), 124.0));

    QCOMPARE(popoverSlot->parentItem()->objectName(), QStringLiteral("surfaceContent"));
    QVERIFY(fuzzyEqual(popoverSlot->parentItem()->x(), 18.0));
    QVERIFY(fuzzyEqual(popoverSlot->parentItem()->y(), 18.0));
    QVERIFY(fuzzyEqual(popoverSlot->parentItem()->width(), 144.0));
    QVERIFY(fuzzyEqual(popoverSlot->parentItem()->height(), 64.0));
}

void SurfaceControlTest::overlayStackUsesSemanticZOrder()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Surfaces

Item {
    Panel { objectName: "panel"; width: 200; height: 100 }
    OscSurface { objectName: "osc"; width: 200; height: 100 }
    Drawer { objectName: "drawer"; width: 200; height: 100 }
    Popover { objectName: "popover"; width: 200; height: 100 }
    Hud { objectName: "hud"; width: 200; height: 100 }
}
)QML"),
        QUrl(QStringLiteral("qrc:/SurfaceZOrderContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    QObject* panel = requireChild(object.get(), "panel");
    QObject* osc = requireChild(object.get(), "osc");
    QObject* drawer = requireChild(object.get(), "drawer");
    QObject* popover = requireChild(object.get(), "popover");
    QObject* hud = requireChild(object.get(), "hud");
    QVERIFY(panel != nullptr);
    QVERIFY(osc != nullptr);
    QVERIFY(drawer != nullptr);
    QVERIFY(popover != nullptr);
    QVERIFY(hud != nullptr);

    const double panelZ = panel->property("z").toDouble();
    const double oscZ = osc->property("z").toDouble();
    const double drawerZ = drawer->property("z").toDouble();
    const double popoverZ = popover->property("z").toDouble();
    const double hudZ = hud->property("z").toDouble();
    QVERIFY(panelZ < oscZ);
    QVERIFY(oscZ < drawerZ);
    QVERIFY(drawerZ < popoverZ);
    QVERIFY(popoverZ < hudZ);
}

void SurfaceControlTest::surfaceModuleRemainsPresentationOnly()
{
    const QStringList violations = surfaceImportViolations();
    QVERIFY2(
        violations.isEmpty(),
        qPrintable(violations.join(QLatin1Char('\n'))));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::SurfaceControlTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "surface_control_test.moc"
