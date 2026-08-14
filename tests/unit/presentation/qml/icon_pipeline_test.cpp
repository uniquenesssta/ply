#include <QColor>
#include <QDir>
#include <QDirIterator>
#include <QFile>
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

QStringList iconPipelineBypassViolations()
{
    const QStringList roots = {
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/shell"),
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/screens"),
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/features"),
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/controls"),
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/surfaces"),
    };

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
                if (!lines.at(index).contains(QStringLiteral("assets/icons/"))) {
                    continue;
                }
                violations.append(
                    QStringLiteral("%1:%2 bypasses the Icon primitive")
                        .arg(filePath)
                        .arg(index + 1));
            }
        }
    }

    return violations;
}

} // namespace

class IconPipelineTest final : public QObject
{
    Q_OBJECT

private slots:
    void currentFigmaIconsLoad();
    void missingIconIsDiagnosable();
    void assetInventoryMatchesContract();
    void productQmlUsesIconPrimitive();
};

void IconPipelineTest::currentFigmaIconsLoad()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    const QByteArray source = R"QML(
import QtQuick
import Player.Presentation.Primitives

Item {
    Icon { id: previousIcon; iconId: "previous" }
    Icon { id: playIcon; iconId: "play" }
    Icon { id: nextIcon; iconId: "next" }
    Icon { id: volumeIcon; iconId: "volume" }
    Icon { id: subtitlesIcon; iconId: "subtitles" }
    Icon { id: playlistIcon; iconId: "playlist" }
    Icon { id: fullscreenIcon; iconId: "fullscreen" }
    Icon { id: minimizeIcon; iconId: "minimize" }
    Icon { id: maximizeIcon; iconId: "maximize" }
    Icon { id: restoreIcon; iconId: "restore" }
    Icon { id: closeIcon; iconId: "close" }
    Icon { id: searchIcon; iconId: "search" }

    readonly property bool allKnown: previousIcon.known
                                     && playIcon.known
                                     && nextIcon.known
                                     && volumeIcon.known
                                     && subtitlesIcon.known
                                     && playlistIcon.known
                                     && fullscreenIcon.known
                                     && minimizeIcon.known
                                     && maximizeIcon.known
                                     && restoreIcon.known
                                     && closeIcon.known
                                     && searchIcon.known
    readonly property bool allReady: previousIcon.ready
                                     && playIcon.ready
                                     && nextIcon.ready
                                     && volumeIcon.ready
                                     && subtitlesIcon.ready
                                     && playlistIcon.ready
                                     && fullscreenIcon.ready
                                     && minimizeIcon.ready
                                     && maximizeIcon.ready
                                     && restoreIcon.ready
                                     && closeIcon.ready
                                     && searchIcon.ready
    readonly property color previousColor: previousIcon.color
    readonly property color searchColor: searchIcon.color
    readonly property color closeColor: closeIcon.color
    readonly property color minimizeColor: minimizeIcon.color
    readonly property color maximizeColor: maximizeIcon.color
    readonly property color restoreColor: restoreIcon.color
    readonly property real previousImplicitWidth: previousIcon.implicitWidth
    readonly property real playImplicitWidth: playIcon.implicitWidth
}
)QML";

    component.setData(
        source,
        QUrl(QStringLiteral("qrc:/IconPipelineContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString loadDiagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(loadDiagnostics));
    QVERIFY2(component.isReady(), qPrintable(loadDiagnostics));

    std::unique_ptr<QObject> object(component.create());
    const QString createDiagnostics = componentDiagnostics(component);
    QVERIFY2(object != nullptr, qPrintable(createDiagnostics));

    QVERIFY(object->property("allKnown").toBool());
    QTRY_VERIFY_WITH_TIMEOUT(object->property("allReady").toBool(), 5000);

    const QColor primary(QStringLiteral("#1A1720"));
    const QColor secondary(QStringLiteral("#76707B"));
    QCOMPARE(object->property("previousColor").value<QColor>(), primary);
    QCOMPARE(object->property("searchColor").value<QColor>(), secondary);
    QCOMPARE(object->property("closeColor").value<QColor>(), secondary);
    QCOMPARE(object->property("minimizeColor").value<QColor>(), secondary);
    QCOMPARE(object->property("maximizeColor").value<QColor>(), secondary);
    QCOMPARE(object->property("restoreColor").value<QColor>(), secondary);
    QCOMPARE(object->property("previousImplicitWidth").toReal(), qreal(22.0));
    QCOMPARE(object->property("playImplicitWidth").toReal(), qreal(24.0));
}

void IconPipelineTest::missingIconIsDiagnosable()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);

    component.setData(
        QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Primitives

Icon {
    iconId: "not-in-catalog"
}
)QML"),
        QUrl(QStringLiteral("qrc:/MissingIconContract.qml")));

    const bool resolved = waitForComponentResolution(component);
    const QString loadDiagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(loadDiagnostics));
    QVERIFY2(component.isReady(), qPrintable(loadDiagnostics));

    std::unique_ptr<QObject> object(component.create());
    const QString createDiagnostics = componentDiagnostics(component);
    QVERIFY2(object != nullptr, qPrintable(createDiagnostics));

    QVERIFY(!object->property("known").toBool());
    QVERIFY(!object->property("ready").toBool());
    QVERIFY(
        object->property("diagnostic")
            .toString()
            .contains(QStringLiteral("Unknown icon id")));
}

void IconPipelineTest::assetInventoryMatchesContract()
{
    QDir iconDirectory(
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/assets/icons"));
    QVERIFY2(iconDirectory.exists(), qPrintable(iconDirectory.absolutePath()));

    const QStringList actual = iconDirectory.entryList(
        {QStringLiteral("*.svg")},
        QDir::Files,
        QDir::Name);
    const QStringList expected = {
        QStringLiteral("close.svg"),
        QStringLiteral("fullscreen.svg"),
        QStringLiteral("maximize.svg"),
        QStringLiteral("minimize.svg"),
        QStringLiteral("next.svg"),
        QStringLiteral("play.svg"),
        QStringLiteral("playlist.svg"),
        QStringLiteral("previous.svg"),
        QStringLiteral("restore.svg"),
        QStringLiteral("search.svg"),
        QStringLiteral("subtitles.svg"),
        QStringLiteral("volume.svg"),
    };

    QCOMPARE(actual, expected);
}

void IconPipelineTest::productQmlUsesIconPrimitive()
{
    const QStringList violations = iconPipelineBypassViolations();
    QVERIFY2(
        violations.isEmpty(),
        qPrintable(violations.join(QLatin1Char('\n'))));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::IconPipelineTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "icon_pipeline_test.moc"
