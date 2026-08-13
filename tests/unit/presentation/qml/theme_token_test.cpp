#include <QColor>
#include <QDirIterator>
#include <QFile>
#include <QFont>
#include <QGuiApplication>
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

QStringList styleLiteralViolations()
{
    const QStringList roots = {
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/shell"),
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/screens"),
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/features"),
    };

    const QRegularExpression colorLiteral(
        QStringLiteral(R"(#[0-9A-Fa-f]{6}(?:[0-9A-Fa-f]{2})?\b)"));
    const QRegularExpression metricLiteral(
        QStringLiteral(
            R"(^\s*(?:width|height|minimumWidth|minimumHeight|implicitWidth|implicitHeight|leftMargin|rightMargin|topMargin|bottomMargin|margins|spacing|padding|font\.pixelSize)\s*:\s*-?\d+(?:\.\d+)?\b)"));

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
                const QString& line = lines.at(index);
                if (colorLiteral.match(line).hasMatch()) {
                    violations.append(
                        QStringLiteral("%1:%2 contains a raw color literal")
                            .arg(filePath)
                            .arg(index + 1));
                }
                if (metricLiteral.match(line).hasMatch()) {
                    violations.append(
                        QStringLiteral("%1:%2 contains a raw visual metric")
                            .arg(filePath)
                            .arg(index + 1));
                }
            }
        }
    }

    return violations;
}

} // namespace

class ThemeTokenTest final : public QObject
{
    Q_OBJECT

private slots:
    void semanticTokensResolve();
    void coreQmlUsesSemanticTokens();
};

void ThemeTokenTest::semanticTokensResolve()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    const QByteArray source = R"QML(
import QtQuick
import Player.Presentation.Theme

QtObject {
    readonly property color surfaceCanvas: ColorTokens.surfaceCanvas
    readonly property color surfaceVideo: ColorTokens.surfaceVideo
    readonly property color textPrimary: ColorTokens.textPrimary
    readonly property color accentPrimary: ColorTokens.accentPrimary
    readonly property color legacyWindowBackground: Theme.windowBackground
    readonly property string cjkFamily: TypographyPrimitives.cjkFamily
    readonly property string monoFamily: TypographyPrimitives.monoFamily
    readonly property font mediaTitleFont: TypographyTokens.mediaTitle
    readonly property int headerContentSpacing: SpacingTokens.headerContent
    readonly property int oscBottomSpacing: SpacingTokens.oscBottom
    readonly property int headerHeight: LayoutTokens.headerHeight
    readonly property int oscHeight: LayoutTokens.oscHeight
    readonly property int referenceWidth: LayoutTokens.windowReferenceWidth
}
)QML";

    component.setData(
        source,
        QUrl(QStringLiteral("qrc:/ThemeTokenContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString loadDiagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(loadDiagnostics));
    QVERIFY2(component.isReady(), qPrintable(loadDiagnostics));

    std::unique_ptr<QObject> object(component.create());
    const QString createDiagnostics = componentDiagnostics(component);
    QVERIFY2(object != nullptr, qPrintable(createDiagnostics));

    QCOMPARE(object->property("surfaceCanvas").value<QColor>(), QColor(QStringLiteral("#F7F7FC")));
    QCOMPARE(object->property("surfaceVideo").value<QColor>(), QColor(QStringLiteral("#EDEEF7")));
    QCOMPARE(object->property("textPrimary").value<QColor>(), QColor(QStringLiteral("#1A1720")));
    QCOMPARE(object->property("accentPrimary").value<QColor>(), QColor(QStringLiteral("#7B2CFF")));
    QCOMPARE(
        object->property("legacyWindowBackground").value<QColor>(),
        object->property("surfaceCanvas").value<QColor>());

    QCOMPARE(object->property("cjkFamily").toString(), QStringLiteral("Noto Sans SC"));
    QCOMPARE(object->property("monoFamily").toString(), QStringLiteral("Geist Mono"));

    const QFont mediaTitleFont = object->property("mediaTitleFont").value<QFont>();
    QCOMPARE(mediaTitleFont.pixelSize(), 14);
    QCOMPARE(
        static_cast<int>(mediaTitleFont.weight()),
        static_cast<int>(QFont::Medium));

    QCOMPARE(object->property("headerContentSpacing").toInt(), 22);
    QCOMPARE(object->property("oscBottomSpacing").toInt(), 38);
    QCOMPARE(object->property("headerHeight").toInt(), 54);
    QCOMPARE(object->property("oscHeight").toInt(), 124);
    QCOMPARE(object->property("referenceWidth").toInt(), 1320);
}

void ThemeTokenTest::coreQmlUsesSemanticTokens()
{
    const QStringList violations = styleLiteralViolations();
    QVERIFY2(
        violations.isEmpty(),
        qPrintable(violations.join(QLatin1Char('\n'))));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::ThemeTokenTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "theme_token_test.moc"
