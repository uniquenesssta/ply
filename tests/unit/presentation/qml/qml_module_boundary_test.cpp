#include <QColor>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QRegularExpression>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <QtTest>

#include <memory>

namespace player::presentation::qml {
namespace {

QString componentDiagnostics(const QQmlComponent& component)
{
    QStringList diagnostics;
    for (const QQmlError& error : component.errors()) {
        diagnostics.append(error.toString());
    }
    return diagnostics.join(QLatin1Char('\n'));
}

QStringList featureImportViolations()
{
    QStringList violations;
    const QString featureRoot =
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/features");
    QDirIterator iterator(
        featureRoot,
        {QStringLiteral("*.qml")},
        QDir::Files,
        QDirIterator::Subdirectories);

    const QRegularExpression pathImport(
        QStringLiteral(R"(^\s*import\s+[\"'])"));
    const QRegularExpression moduleImport(
        QStringLiteral(R"(^\s*import\s+([A-Za-z_][A-Za-z0-9_.]*))"));

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
            if (pathImport.match(line).hasMatch()) {
                violations.append(
                    QStringLiteral("%1:%2 uses a path import")
                        .arg(filePath)
                        .arg(index + 1));
                continue;
            }

            const QRegularExpressionMatch match = moduleImport.match(line);
            if (!match.hasMatch()) {
                continue;
            }

            const QString module = match.captured(1);
            if (!module.startsWith(QStringLiteral("Player.Presentation."))) {
                continue;
            }

            if (module != QStringLiteral("Player.Presentation.Theme")
                && module != QStringLiteral("Player.Presentation.Controls")) {
                violations.append(
                    QStringLiteral("%1:%2 imports non-public design module %3")
                        .arg(filePath)
                        .arg(index + 1)
                        .arg(module));
            }
        }
    }

    return violations;
}

} // namespace

class QmlModuleBoundaryTest final : public QObject
{
    Q_OBJECT

private slots:
    void publicDesignSystemModulesLoad();
    void featureImportsUsePublicDesignModulesOnly();
};

void QmlModuleBoundaryTest::publicDesignSystemModulesLoad()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    const QByteArray source = R"QML(
import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Primitives
import Player.Presentation.Controls
import Player.Presentation.Surfaces

QtObject {
    readonly property color themeColor: Theme.windowBackground
}
)QML";

    component.setData(
        source,
        QUrl(QStringLiteral("inmemory:/DesignSystemModuleSmoke.qml")));

    const QString loadDiagnostics = componentDiagnostics(component);
    QVERIFY2(component.isReady(), qPrintable(loadDiagnostics));

    std::unique_ptr<QObject> object(component.create());
    const QString createDiagnostics = componentDiagnostics(component);
    QVERIFY2(object != nullptr, qPrintable(createDiagnostics));

    const QVariant themeColor = object->property("themeColor");
    QVERIFY(themeColor.isValid());
    QVERIFY(themeColor.value<QColor>().isValid());
}

void QmlModuleBoundaryTest::featureImportsUsePublicDesignModulesOnly()
{
    const QStringList violations = featureImportViolations();
    QVERIFY2(
        violations.isEmpty(),
        qPrintable(violations.join(QLatin1Char('\n'))));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::QmlModuleBoundaryTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "qml_module_boundary_test.moc"
