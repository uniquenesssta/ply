#include <QColor>
#include <QFont>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
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

class TypographyPrimitiveTest final : public QObject
{
    Q_OBJECT

private slots:
    void semanticStylesResolve();
    void longTitleElides();
    void timecodeUsesMonospaceContract();
};

void TypographyPrimitiveTest::semanticStylesResolve()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Primitives

Item {
    TitleText { id: inspectorTitle; variant: TitleText.Inspector }
    TitleText { id: mediaCompact; variant: TitleText.MediaCompact }
    BodyText { id: supporting; variant: BodyText.Supporting }
    BodyText { id: control; variant: BodyText.Control }
    CaptionText { id: meta; variant: CaptionText.Meta }
    CaptionText { id: technical; variant: CaptionText.Technical }
    CaptionText { id: compactStrong; variant: CaptionText.CompactStrong }
    TimecodeText { id: timecode; variant: TimecodeText.MediumPrimary; text: "01:23:45" }

    readonly property font inspectorFont: inspectorTitle.font
    readonly property font compactMediaFont: mediaCompact.font
    readonly property font supportingFont: supporting.font
    readonly property font controlFont: control.font
    readonly property font metaFont: meta.font
    readonly property font technicalFont: technical.font
    readonly property font compactStrongFont: compactStrong.font
    readonly property color titleColor: inspectorTitle.color
    readonly property color bodyColor: control.color
    readonly property color captionColor: meta.color
}
)QML"),
        QUrl(QStringLiteral("qrc:/TypographyPrimitiveContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    const QFont inspectorFont = object->property("inspectorFont").value<QFont>();
    QCOMPARE(inspectorFont.pixelSize(), 22);
    QCOMPARE(static_cast<int>(inspectorFont.weight()), static_cast<int>(QFont::Medium));

    const QFont compactMediaFont = object->property("compactMediaFont").value<QFont>();
    QCOMPARE(compactMediaFont.pixelSize(), 13);
    QCOMPARE(static_cast<int>(compactMediaFont.weight()), static_cast<int>(QFont::Medium));

    const QFont supportingFont = object->property("supportingFont").value<QFont>();
    QCOMPARE(supportingFont.pixelSize(), 14);
    QCOMPARE(static_cast<int>(supportingFont.weight()), static_cast<int>(QFont::Normal));

    const QFont controlFont = object->property("controlFont").value<QFont>();
    QCOMPARE(controlFont.pixelSize(), 12);

    const QFont metaFont = object->property("metaFont").value<QFont>();
    QCOMPARE(metaFont.pixelSize(), 11);

    const QFont technicalFont = object->property("technicalFont").value<QFont>();
    QCOMPARE(technicalFont.pixelSize(), 10);

    const QFont compactStrongFont = object->property("compactStrongFont").value<QFont>();
    QCOMPARE(compactStrongFont.pixelSize(), 11);
    QCOMPARE(static_cast<int>(compactStrongFont.weight()), static_cast<int>(QFont::DemiBold));

    QCOMPARE(
        object->property("titleColor").value<QColor>(),
        QColor(QStringLiteral("#1A1720")));
    QCOMPARE(
        object->property("bodyColor").value<QColor>(),
        QColor(QStringLiteral("#706A78")));
    QCOMPARE(
        object->property("captionColor").value<QColor>(),
        QColor(QStringLiteral("#76707B")));
}

void TypographyPrimitiveTest::longTitleElides()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Primitives

Item {
    width: 240
    height: 80

    TitleText {
        id: title
        width: 96
        text: "A deliberately very long media title that must be elided"
        variant: TitleText.Media
    }

    readonly property bool titleTruncated: title.truncated
    readonly property int titleLineCount: title.lineCount
}
)QML"),
        QUrl(QStringLiteral("qrc:/TypographyLongTitleContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    QTRY_VERIFY_WITH_TIMEOUT(object->property("titleTruncated").toBool(), 2000);
    QCOMPARE(object->property("titleLineCount").toInt(), 1);
}

void TypographyPrimitiveTest::timecodeUsesMonospaceContract()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Primitives

TimecodeText {
    text: "01:23:45"
    variant: TimecodeText.MediumPrimary
    readonly property string requestedFamily: font.family
    readonly property int requestedPixelSize: font.pixelSize
    readonly property int requestedWeight: font.weight
    readonly property int renderedLineCount: lineCount
}
)QML"),
        QUrl(QStringLiteral("qrc:/TypographyTimecodeContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString diagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(diagnostics));
    QVERIFY2(component.isReady(), qPrintable(diagnostics));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(componentDiagnostics(component)));

    QCOMPARE(object->property("text").toString(), QStringLiteral("01:23:45"));
    QCOMPARE(object->property("requestedFamily").toString(), QStringLiteral("Geist Mono"));
    QCOMPARE(object->property("requestedPixelSize").toInt(), 12);
    QCOMPARE(
        object->property("requestedWeight").toInt(),
        static_cast<int>(QFont::Medium));
    QCOMPARE(object->property("renderedLineCount").toInt(), 1);
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::TypographyPrimitiveTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "typography_primitive_test.moc"
