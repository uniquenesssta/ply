#include "app/composition/application_container.h"

#include "app/bootstrap/logging_bootstrap.h"
#include "app/composition/playback_composition.h"
#include "media/application/open/media_open_coordinator.h"
#include "presentation/viewmodels/player/hud/hud_message_queue.h"
#include "presentation/viewmodels/player/status/player_status_view_model.h"
#include "presentation/viewmodels/player/transport/player_transport_view_model.h"
#include "presentation/viewmodels/player/volume/player_volume_view_model.h"

#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

#include <memory>
#include <utility>

namespace player::app {
namespace {

QStringList sessionLogFiles(const QString& directoryPath)
{
    return QDir(directoryPath).entryList(
        {QStringLiteral("player-*.log")},
        QDir::Files,
        QDir::Name);
}

bool writeSilentWav(const QString& path)
{
    constexpr quint32 sampleRate = 8000;
    constexpr quint16 channels = 1;
    constexpr quint16 bitsPerSample = 16;
    constexpr quint32 durationSeconds = 2;
    constexpr quint16 blockAlign = channels * (bitsPerSample / 8);
    constexpr quint32 byteRate = sampleRate * blockAlign;
    constexpr quint32 dataSize = byteRate * durationSeconds;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    if (stream.writeRawData("RIFF", 4) != 4) {
        return false;
    }
    stream << quint32(36 + dataSize);
    if (stream.writeRawData("WAVE", 4) != 4
        || stream.writeRawData("fmt ", 4) != 4) {
        return false;
    }
    stream << quint32(16);
    stream << quint16(1);
    stream << channels;
    stream << sampleRate;
    stream << byteRate;
    stream << blockAlign;
    stream << bitsPerSample;
    if (stream.writeRawData("data", 4) != 4) {
        return false;
    }
    stream << dataSize;

    const QByteArray silence(static_cast<qsizetype>(dataSize), '\0');
    return file.write(silence) == silence.size();
}

} // namespace

class ApplicationContainerTest final : public QObject
{
    Q_OBJECT

private slots:
    void ownsResolvedRuntimePathsByValue();
    void loggingBootstrapCreatesResolvedDevelopmentLog();
    void adoptsPreStartedLoggingBootstrap();
    void ownsMediaOpenCoordinator();
    void playbackCompositionStartsAndStops();
    void mediaOpenCoordinatorLoadsLocalWav();
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
    const QString markerDirectory = QDir(executableDirectory).filePath(
        QStringLiteral("cmake"));
    QVERIFY(QDir().mkpath(markerDirectory));

    QFile developmentMarker(QDir(markerDirectory).filePath(
        QStringLiteral(".player-development-root")));
    QVERIFY(developmentMarker.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QCOMPARE(developmentMarker.write("../../..\n"), qint64(9));
    developmentMarker.close();

    ApplicationContainer container(RuntimePaths::resolve(
        RuntimePaths::Mode::Installed,
        executableDirectory));

    const QString expectedLogDirectory = QDir(temporaryDirectory.path()).filePath(
        QStringLiteral("logs"));
    QCOMPARE(
        QDir::cleanPath(container.runtimePaths().logDirectory()),
        QDir::cleanPath(expectedLogDirectory));

    QString error;
    QVERIFY2(
        container.loggingBootstrap().start(container.runtimePaths(), &error),
        qPrintable(error));

    const QStringList logs = sessionLogFiles(expectedLogDirectory);
    QCOMPARE(logs.size(), 1);
    QVERIFY2(
        QFileInfo(QDir(expectedLogDirectory).filePath(logs.constFirst())).isFile(),
        qPrintable(expectedLogDirectory));

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
    QCOMPARE(sessionLogFiles(runtimePaths.logDirectory()).size(), 1);

    ApplicationContainer container(runtimePaths, std::move(loggingBootstrap));

    error.clear();
    QVERIFY2(
        container.loggingBootstrap().start(container.runtimePaths(), &error),
        qPrintable(error));
    QCOMPARE(sessionLogFiles(runtimePaths.logDirectory()).size(), 1);

    container.shutdown();
}

void ApplicationContainerTest::ownsMediaOpenCoordinator()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    ApplicationContainer container(RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        temporaryDirectory.path()));

    QCOMPARE(container.mediaOpenCoordinator().lastErrorKey(), QString{});
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
    QVERIFY(playback.statusViewModel().visible());
    QVERIFY(!playback.statusViewModel().errorVisible());
    QCOMPARE(playback.statusViewModel().statusKey(), QStringLiteral("empty"));
    QVERIFY(!playback.hudMessageQueue().visible());
    QCOMPARE(playback.hudMessageQueue().messageKey(), QString{});

    QString error;
    QVERIFY2(playback.start(&error), qPrintable(error));
    QTRY_VERIFY_WITH_TIMEOUT(playback.isRunning(), 1000);

    error.clear();
    QVERIFY2(playback.stop(&error), qPrintable(error));
    QVERIFY(!playback.isRunning());
    QVERIFY(!playback.hudMessageQueue().visible());
}

void ApplicationContainerTest::mediaOpenCoordinatorLoadsLocalWav()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString wavPath = temporaryDirectory.filePath(QStringLiteral("local fixture.wav"));
    QVERIFY(writeSilentWav(wavPath));

    ApplicationContainer container(RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        temporaryDirectory.path()));
    PlaybackComposition& playback = container.playbackComposition();

    QString error;
    QVERIFY2(playback.start(&error), qPrintable(error));
    QTRY_VERIFY_WITH_TIMEOUT(playback.isRunning(), 1000);

    QVERIFY(container.mediaOpenCoordinator().openLocalFile(QUrl::fromLocalFile(wavPath)));
    QTRY_VERIFY_WITH_TIMEOUT(
        playback.transportViewModel().canPlay()
            || playback.transportViewModel().canPause()
            || playback.transportViewModel().canStop(),
        5000);
    QVERIFY_NE(playback.statusViewModel().statusKey(), QStringLiteral("empty"));

    error.clear();
    QVERIFY2(playback.stop(&error), qPrintable(error));
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
