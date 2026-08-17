#include <QColor>
#include <QDirIterator>
#include <QFile>
#include <QFont>
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
    void globalDarkModeResolvesSharedSemanticTokens();
    void darkModeIsGlobalNotFeatureScoped();
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

void ThemeTokenTest::globalDarkModeResolvesSharedSemanticTokens()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    const QByteArray source = R"QML(
import QtQuick
import Player.Presentation.Theme

QtObject {
    readonly property color surfaceCanvas: ColorTokens.surfaceCanvas
    readonly property color surfaceGlass: ColorTokens.surfaceGlass
    readonly property color surfaceGlassStrong: ColorTokens.surfaceGlassStrong
    readonly property color surfaceInspector: ColorTokens.surfaceInspector
    readonly property color surfaceInspectorSearch: ColorTokens.surfaceInspectorSearch
    readonly property color surfaceInspectorRow: ColorTokens.surfaceInspectorRow
    readonly property color surfaceInspectorSelection: ColorTokens.surfaceInspectorSelection
    readonly property color textPrimary: ColorTokens.textPrimary
    readonly property color textSecondary: ColorTokens.textSecondary
    readonly property color accentPrimary: ColorTokens.accentPrimary
    readonly property color accentStrong: ColorTokens.accentStrong
    readonly property color controlProgress: ColorTokens.controlProgress
    readonly property real headerFillAlpha: MaterialTokens.headerFillAlpha
    readonly property real oscFillAlpha: MaterialTokens.oscFillAlpha
    readonly property real inspectorFillAlpha: MaterialTokens.inspectorFillAlpha
    readonly property real rowFillAlpha: MaterialTokens.rowFillAlpha
    readonly property real borderSoftAlpha: MaterialTokens.borderSoftAlpha

    function useDark() {
        return ThemeMode.setMode(ThemeMode.darkMode)
    }
}
)QML";

    component.setData(source, QUrl(QStringLiteral("qrc:/GlobalDarkThemeContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString loadDiagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(loadDiagnostics));
    QVERIFY2(component.isReady(), qPrintable(loadDiagnostics));

    std::unique_ptr<QObject> object(component.create());
    const QString createDiagnostics = componentDiagnostics(component);
    QVERIFY2(object != nullptr, qPrintable(createDiagnostics));

    QCOMPARE(object->property("surfaceCanvas").value<QColor>(), QColor(QStringLiteral("#F7F7FC")));
    QVERIFY(QMetaObject::invokeMethod(object.get(), "useDark"));

    QTRY_COMPARE(object->property("surfaceCanvas").value<QColor>(), QColor(QStringLiteral("#17141F")));
    QCOMPARE(object->property("surfaceGlass").value<QColor>(), QColor(QStringLiteral("#211B29")));
    QCOMPARE(object->property("surfaceGlassStrong").value<QColor>(), QColor(QStringLiteral("#27212F")));
    QCOMPARE(object->property("surfaceInspector").value<QColor>(), QColor(QStringLiteral("#17141F")));
    QCOMPARE(object->property("surfaceInspectorSearch").value<QColor>(), QColor(QStringLiteral("#27212F")));
    QCOMPARE(object->property("surfaceInspectorRow").value<QColor>(), QColor(QStringLiteral("#211B29")));
    QCOMPARE(object->property("surfaceInspectorSelection").value<QColor>(), QColor(QStringLiteral("#3A2852")));
    QCOMPARE(object->property("textPrimary").value<QColor>(), QColor(QStringLiteral("#F7F4FB")));
    QCOMPARE(object->property("textSecondary").value<QColor>(), QColor(QStringLiteral("#B6ADBC")));
    QCOMPARE(object->property("accentPrimary").value<QColor>(), QColor(QStringLiteral("#A879FF")));
    QCOMPARE(object->property("accentStrong").value<QColor>(), QColor(QStringLiteral("#C49AFF")));
    QCOMPARE(object->property("controlProgress").value<QColor>(), QColor(QStringLiteral("#A879FF")));
    QCOMPARE(object->property("headerFillAlpha").toDouble(), 1.0);
    QCOMPARE(object->property("oscFillAlpha").toDouble(), 1.0);
    QCOMPARE(object->property("inspectorFillAlpha").toDouble(), 1.0);
    QCOMPARE(object->property("rowFillAlpha").toDouble(), 1.0);
    QCOMPARE(object->property("borderSoftAlpha").toDouble(), 1.0);
}

void ThemeTokenTest::darkModeIsGlobalNotFeatureScoped()
{
    const QString themeCMake = readSource(QStringLiteral(
        "src/presentation/qml/theme/CMakeLists.txt"));
    const QString mode = readSource(QStringLiteral(
        "src/presentation/qml/theme/ThemeMode.qml"));
    const QString colors = readSource(QStringLiteral(
        "src/presentation/qml/theme/ColorTokens.qml"));
    const QString materials = readSource(QStringLiteral(
        "src/presentation/qml/theme/MaterialTokens.qml"));
    const QString drawer = readSource(QStringLiteral(
        "src/presentation/qml/surfaces/Drawer.qml"));
    const QString shell = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/PlayerInspectorShell.qml"));
    const QString search = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/PlaylistSearchField.qml"));
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/playlist/PlaylistRow.qml"));

    QVERIFY(themeCMake.contains(QStringLiteral("ThemeMode.qml")));
    QVERIFY(mode.contains(QStringLiteral("property int mode: lightMode")));
    QVERIFY(mode.contains(QStringLiteral("readonly property bool isDark")));
    QVERIFY(colors.contains(QStringLiteral("ThemeMode.isDark")));
    QVERIFY(materials.contains(QStringLiteral("ThemeMode.isDark")));
    QVERIFY(drawer.contains(QStringLiteral("ColorTokens.surfaceInspector")));
    QVERIFY(search.contains(QStringLiteral("ColorTokens.surfaceInspectorSearch")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.surfaceInspectorRow")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.surfaceInspectorSelection")));

    const QStringList forbiddenFeatureDarkTokens{
        QStringLiteral("surfaceInspectorDark"),
        QStringLiteral("textInspector"),
        QStringLiteral("accentInspector"),
        QStringLiteral("borderInspector"),
        QStringLiteral("inspectorDark"),
    };
    for (const QString& token : forbiddenFeatureDarkTokens) {
        QVERIFY2(!colors.contains(token), qPrintable(token));
        QVERIFY2(!materials.contains(token), qPrintable(token));
        QVERIFY2(!drawer.contains(token), qPrintable(token));
        QVERIFY2(!shell.contains(token), qPrintable(token));
        QVERIFY2(!search.contains(token), qPrintable(token));
        QVERIFY2(!row.contains(token), qPrintable(token));
    }
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
