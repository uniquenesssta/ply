#include <QColor>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QStringList>
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

} // namespace

class QmlModuleBoundaryTest final : public QObject
{
    Q_OBJECT

private slots:
    void publicDesignSystemModulesLoad();
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

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::QmlModuleBoundaryTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "qml_module_boundary_test.moc"
