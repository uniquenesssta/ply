#include "app/composition/application_container.h"

#include "app/bootstrap/logging_bootstrap.h"
#include "app/composition/playback_composition.h"
#include "presentation/viewmodels/player/transport/player_transport_view_model.h"
#include "presentation/viewmodels/player/volume/player_volume_view_model.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

#include <memory>
#include <utility>

namespace player::app {

class ApplicationContainerTest final : public QObject
{
    Q_OBJECT

private slots:
    void ownsResolvedRuntimePathsByValue();
    void loggingBootstrapCreatesResolvedDevelopmentLog();
    void adoptsPreStartedLoggingBootstrap();
    void playbackCompositionStartsAndStops();
    void shutdownIsIdempotent();
};

void ApplicationContainerTest::ownsResolvedRuntimePathsByValue()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const RuntimePaths expected = RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        temporaryDirectory.path());

    ApplicationContainer container(expected);

    QCOMPARE(container.runtimePaths().mode(), expected.mode());
    QCOMPARE(container.runtimePaths().executableDirectory(), expected.executableDirectory());
    QCOMPARE(container.runtimePaths().configDirectory(), expected.configDirectory());
    QCOMPARE(container.runtimePaths().dataDirectory(), expected.dataDirectory());
    QCOMPARE(container.runtimePaths().logDirectory(), expected.logDirectory());
    QCOMPARE(container.runtimePaths().screenshotDirectory(), expected.screenshotDirectory());
}

void ApplicationContainerTest::loggingBootstrapCreatesResolvedDevelopmentLog()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString projectDirectory = QDir(temporaryDirectory.path()).filePath(
        QStringLiteral("project root with spaces"));
    const QString executableDirectory = QDir(projectDirectory).filePath(
        QStringLiteral("build/custom-output"));
    QVERIFY(QDir().mkpath(executableDirectory));

    QFile developmentMarker(QDir(executableDirectory).filePath(
        QStringLiteral(".player-development-root")));
    QVERIFY(developmentMarker.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QCOMPARE(developmentMarker.write("../..\n"), qint64(6));
    developmentMarker.close();

    ApplicationContainer container(RuntimePaths::resolve(
        RuntimePaths::Mode::Installed,
        executableDirectory));

    QCOMPARE(
        QDir::cleanPath(container.runtimePaths().logDirectory()),
        QDir::cleanPath(projectDirectory));

    QString error;
    QVERIFY2(
        container.loggingBootstrap().start(container.runtimePaths(), &error),
        qPrintable(error));

    const QString logPath = QDir(projectDirectory).filePath(QStringLiteral("player.log"));
    QVERIFY2(QFileInfo(logPath).isFile(), qPrintable(logPath));

    container.shutdown();
}

void ApplicationContainerTest::adoptsPreStartedLoggingBootstrap()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const RuntimePaths runtimePaths = RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        temporaryDirectory.path());
    auto loggingBootstrap = std::make_unique<LoggingBootstrap>();

    QString error;
    QVERIFY2(loggingBootstrap->start(runtimePaths, &error), qPrintable(error));
    QVERIFY2(
        QFileInfo(QDir(runtimePaths.logDirectory()).filePath(QStringLiteral("player.log"))).isFile(),
        qPrintable(runtimePaths.logDirectory()));

    ApplicationContainer container(runtimePaths, std::move(loggingBootstrap));

    error.clear();
    QVERIFY2(
        container.loggingBootstrap().start(container.runtimePaths(), &error),
        qPrintable(error));

    container.shutdown();
}

void ApplicationContainerTest::playbackCompositionStartsAndStops()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    ApplicationContainer container(RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        temporaryDirectory.path()));
    PlaybackComposition& playback = container.playbackComposition();

    QVERIFY(!playback.isRunning());
    QVERIFY(!playback.transportViewModel().canPlay());
    QVERIFY(!playback.transportViewModel().canPause());
    QVERIFY(!playback.transportViewModel().canStop());
    QVERIFY(!playback.volumeViewModel().canAdjustVolume());
    QVERIFY(!playback.volumeViewModel().canToggleMute());

    QString error;
    QVERIFY2(playback.start(&error), qPrintable(error));
    QTRY_VERIFY_WITH_TIMEOUT(playback.isRunning(), 1000);

    error.clear();
    QVERIFY2(playback.stop(&error), qPrintable(error));
    QVERIFY(!playback.isRunning());
}

void ApplicationContainerTest::shutdownIsIdempotent()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    ApplicationContainer container(RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        temporaryDirectory.path()));

    container.shutdown();
    container.shutdown();

    QVERIFY(true);
}

} // namespace player::app

QTEST_GUILESS_MAIN(player::app::ApplicationContainerTest)
#include "application_container_test.moc"
