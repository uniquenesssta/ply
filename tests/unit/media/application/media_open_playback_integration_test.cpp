#include "media/application/open/media_open_coordinator.h"

#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/requests/playback_request_id_generator.h"
#include "playback/application/session/playback_session_thread.h"
#include "playback/application/state_publisher/state_publisher.h"
#include "playback/domain/commands/load_media_command.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

namespace player::media::application {
namespace {

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

class MediaOpenPlaybackIntegrationTest final : public QObject
{
    Q_OBJECT

private slots:
    void localFileReachesRealPlaybackSession();
};

void MediaOpenPlaybackIntegrationTest::localFileReachesRealPlaybackSession()
{
    using player::playback::application::PlaybackCommandBus;
    using player::playback::application::PlaybackRequestIdGenerator;
    using player::playback::application::PlaybackSessionThread;
    using player::playback::application::StatePublisher;
    using player::playback::domain::LoadMediaCommand;
    using player::playback::domain::PlaybackCommand;
    using player::playback::domain::PlaybackLifecycleState;
    using player::playback::domain::PlaybackSnapshot;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString wavPath = directory.filePath(QStringLiteral("local fixture.wav"));
    QVERIFY(writeSilentWav(wavPath));

    PlaybackSessionThread playbackThread;
    PlaybackRequestIdGenerator requestIds;
    QSignalSpy readySpy(&playbackThread, &PlaybackSessionThread::ready);

    bool mediaEstablished = false;
    QObject::connect(
        playbackThread.statePublisher(),
        &StatePublisher::snapshotPublished,
        &playbackThread,
        [&mediaEstablished](const PlaybackSnapshot& snapshot) {
            mediaEstablished = snapshot.lifecycle() == PlaybackLifecycleState::Ready
                || snapshot.lifecycle() == PlaybackLifecycleState::Ended;
        });

    QString startupError;
    QVERIFY2(playbackThread.start(&startupError), qPrintable(startupError));
    QTRY_VERIFY_WITH_TIMEOUT(readySpy.count() > 0, 5000);

    MediaOpenCoordinator coordinator(
        [&playbackThread, &requestIds](const player::media::domain::MediaSource& source) {
            PlaybackCommandBus* bus = playbackThread.commandBus();
            if (bus == nullptr || !bus->isAcceptingCommands()) {
                return false;
            }

            QString diagnostic;
            const PlaybackCommand command{
                requestIds.next(),
                LoadMediaCommand{source.location()}};
            return bus->submit(command, &diagnostic);
        });

    QVERIFY(coordinator.openLocalFile(QUrl::fromLocalFile(wavPath)));
    QTRY_VERIFY_WITH_TIMEOUT(mediaEstablished, 5000);

    QString shutdownError;
    QVERIFY2(playbackThread.stop(&shutdownError), qPrintable(shutdownError));
}

} // namespace player::media::application

QTEST_GUILESS_MAIN(player::media::application::MediaOpenPlaybackIntegrationTest)
#include "media_open_playback_integration_test.moc"
