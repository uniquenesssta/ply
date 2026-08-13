#include <QColor>
#include <QDirIterator>
#include <QFile>
#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QRegularExpression>
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

QStringList effectLiteralViolations()
{
    const QStringList roots = {
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/shell"),
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/screens"),
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/features"),
    };

    const QRegularExpression effectLiteral(
        QStringLiteral(
            R"(^\s*(?:radius|z|opacity|duration)\s*:\s*-?\d+(?:\.\d+)?\b)"));

    QStringList violations;
    for (const QString& root : roots) {
        QDirIterator iterator(
            root,
            {QStringLiteral("*.qml")},
            QDir::Files,
            QDirIterator::Subdirectories);

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
                if (effectLiteral.match(lines.at(index)).hasMatch()) {
                    violations.append(
                        QStringLiteral("%1:%2 contains a raw R5-03 visual token")
                            .arg(filePath)
                            .arg(index + 1));
                }
            }
        }
    }

    return violations;
}

bool fuzzyEquals(double actual, double expected)
{
    return qAbs(actual - expected) < 0.0001;
}

} // namespace

class ThemeEffectTokenTest final : public QObject
{
    Q_OBJECT

private slots:
    void semanticEffectsResolve();
    void reduceMotionPreservesSemanticDelay();
    void coreQmlAvoidsRawEffectMetrics();
};

void ThemeEffectTokenTest::semanticEffectsResolve()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    const QByteArray source = R"QML(
import QtQuick
import Player.Presentation.Theme

QtObject {
    readonly property int radiusWindow: RadiusTokens.windowPlayer
    readonly property int radiusOsc: RadiusTokens.surfaceOsc
    readonly property int radiusInspector: RadiusTokens.surfaceInspector
    readonly property int radiusPopover: RadiusTokens.surfacePopover
    readonly property int radiusDialog: RadiusTokens.surfaceDialog

    readonly property int headerBlur: MaterialTokens.headerBlur
    readonly property int oscBlur: MaterialTokens.oscBlur
    readonly property int inspectorBlur: MaterialTokens.inspectorBlur
    readonly property real headerAlpha: MaterialTokens.headerFillAlpha
    readonly property real oscAlpha: MaterialTokens.oscFillAlpha
    readonly property real inspectorAlpha: MaterialTokens.inspectorFillAlpha

    readonly property color shadowColor: ElevationTokens.shadowColor
    readonly property int floatingShadowRadius: ElevationTokens.floatingShadowRadius
    readonly property int floatingShadowY: ElevationTokens.floatingShadowYOffset
    readonly property real floatingShadowAlpha: ElevationTokens.floatingShadowAlpha
    readonly property int windowShadowRadius: ElevationTokens.windowShadowRadius
    readonly property int windowShadowY: ElevationTokens.windowShadowYOffset

    readonly property real controlIdleOpacity: OpacityTokens.controlIdle
    readonly property real controlDisabledOpacity: OpacityTokens.controlDisabled
    readonly property real dialogScrimOpacity: OpacityTokens.dialogScrim

    readonly property int videoZ: ZOrderTokens.video
    readonly property int headerZ: ZOrderTokens.floatingHeader
    readonly property int oscZ: ZOrderTokens.osc
    readonly property int inspectorZ: ZOrderTokens.inspector
    readonly property int dialogZ: ZOrderTokens.dialog

    readonly property int oscShowDuration: MotionTokens.oscShowDuration
    readonly property int inspectorOpenDuration: MotionTokens.inspectorOpenDuration
    readonly property int toastShowDuration: MotionTokens.toastShowDuration
    readonly property int oscHideDelay: MotionTokens.oscHideDelay
    readonly property int enterBezierLength: MotionTokens.enterBezier.length
    readonly property real enterBezierC1x: MotionTokens.enterBezier[0]
    readonly property real enterBezierC1y: MotionTokens.enterBezier[1]
    readonly property real enterBezierC2x: MotionTokens.enterBezier[2]
    readonly property real enterBezierC2y: MotionTokens.enterBezier[3]
}
)QML";

    component.setData(source, QUrl(QStringLiteral("qrc:/ThemeEffectContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString loadDiagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(loadDiagnostics));
    QVERIFY2(component.isReady(), qPrintable(loadDiagnostics));

    std::unique_ptr<QObject> object(component.create());
    const QString createDiagnostics = componentDiagnostics(component);
    QVERIFY2(object != nullptr, qPrintable(createDiagnostics));

    QCOMPARE(object->property("radiusWindow").toInt(), 32);
    QCOMPARE(object->property("radiusOsc").toInt(), 34);
    QCOMPARE(object->property("radiusInspector").toInt(), 32);
    QCOMPARE(object->property("radiusPopover").toInt(), 20);
    QCOMPARE(object->property("radiusDialog").toInt(), 30);

    QCOMPARE(object->property("headerBlur").toInt(), 28);
    QCOMPARE(object->property("oscBlur").toInt(), 36);
    QCOMPARE(object->property("inspectorBlur").toInt(), 42);
    QVERIFY(fuzzyEquals(object->property("headerAlpha").toDouble(), 0.52));
    QVERIFY(fuzzyEquals(object->property("oscAlpha").toDouble(), 0.34));
    QVERIFY(fuzzyEquals(object->property("inspectorAlpha").toDouble(), 0.38));

    QCOMPARE(
        object->property("shadowColor").value<QColor>(),
        QColor(QStringLiteral("#2E1F47")));
    QCOMPARE(object->property("floatingShadowRadius").toInt(), 42);
    QCOMPARE(object->property("floatingShadowY").toInt(), 12);
    QVERIFY(fuzzyEquals(object->property("floatingShadowAlpha").toDouble(), 0.12));
    QCOMPARE(object->property("windowShadowRadius").toInt(), 70);
    QCOMPARE(object->property("windowShadowY").toInt(), 24);

    QVERIFY(fuzzyEquals(object->property("controlIdleOpacity").toDouble(), 0.72));
    QVERIFY(fuzzyEquals(object->property("controlDisabledOpacity").toDouble(), 0.38));
    QVERIFY(fuzzyEquals(object->property("dialogScrimOpacity").toDouble(), 0.18));

    QCOMPARE(object->property("videoZ").toInt(), 0);
    QCOMPARE(object->property("headerZ").toInt(), 30);
    QCOMPARE(object->property("oscZ").toInt(), 40);
    QCOMPARE(object->property("inspectorZ").toInt(), 50);
    QCOMPARE(object->property("dialogZ").toInt(), 100);

    QCOMPARE(object->property("oscShowDuration").toInt(), 160);
    QCOMPARE(object->property("inspectorOpenDuration").toInt(), 240);
    QCOMPARE(object->property("toastShowDuration").toInt(), 200);
    QCOMPARE(object->property("oscHideDelay").toInt(), 2200);
    QCOMPARE(object->property("enterBezierLength").toInt(), 6);
    QVERIFY(fuzzyEquals(object->property("enterBezierC1x").toDouble(), 0.2));
    QVERIFY(fuzzyEquals(object->property("enterBezierC1y").toDouble(), 0.0));
    QVERIFY(fuzzyEquals(object->property("enterBezierC2x").toDouble(), 0.0));
    QVERIFY(fuzzyEquals(object->property("enterBezierC2y").toDouble(), 1.0));
}

void ThemeEffectTokenTest::reduceMotionPreservesSemanticDelay()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    const QByteArray source = R"QML(
import QtQuick
import Player.Presentation.Theme

QtObject {
    readonly property int oscShowDuration: MotionTokens.oscShowDuration
    readonly property int inspectorOpenDuration: MotionTokens.inspectorOpenDuration
    readonly property int popoverCloseDuration: MotionTokens.popoverCloseDuration
    readonly property int timelineCommitDuration: MotionTokens.timelineCommitDuration
    readonly property int controlStateDuration: MotionTokens.controlStateDuration
    readonly property int oscHideDelay: MotionTokens.oscHideDelay
    readonly property int enterBezierLength: MotionTokens.enterBezier.length

    function enableReduceMotion() {
        MotionTokens.reduceMotionEnabled = true
    }
}
)QML";

    component.setData(source, QUrl(QStringLiteral("qrc:/ReduceMotionContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString loadDiagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(loadDiagnostics));
    QVERIFY2(component.isReady(), qPrintable(loadDiagnostics));

    std::unique_ptr<QObject> object(component.create());
    const QString createDiagnostics = componentDiagnostics(component);
    QVERIFY2(object != nullptr, qPrintable(createDiagnostics));

    QVERIFY(QMetaObject::invokeMethod(object.get(), "enableReduceMotion"));

    QCOMPARE(object->property("oscShowDuration").toInt(), 0);
    QCOMPARE(object->property("inspectorOpenDuration").toInt(), 0);
    QCOMPARE(object->property("popoverCloseDuration").toInt(), 0);
    QCOMPARE(object->property("timelineCommitDuration").toInt(), 0);
    QCOMPARE(object->property("controlStateDuration").toInt(), 0);
    QCOMPARE(object->property("enterBezierLength").toInt(), 0);
    QCOMPARE(object->property("oscHideDelay").toInt(), 2200);
}

void ThemeEffectTokenTest::coreQmlAvoidsRawEffectMetrics()
{
    const QStringList violations = effectLiteralViolations();
    QVERIFY2(
        violations.isEmpty(),
        qPrintable(violations.join(QLatin1Char('\n'))));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::ThemeEffectTokenTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "theme_effect_token_test.moc"
