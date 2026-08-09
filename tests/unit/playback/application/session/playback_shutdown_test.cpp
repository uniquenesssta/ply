#include "fixtures/session_test_media.h"

#include "foundation/ids/request_id.h"
#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/session/playback_session.h"
#include "playback/application/session/playback_session_thread.h"
#include "playback/application/state_publisher/state_publisher.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <QCoreApplication>
#include <QTemporaryDir>

#include <utility>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;

PlaybackCommand makeCommand(quint64 requestId, PlaybackCommandPayload payload)
{
    return PlaybackCommand{player::ids::RequestId{requestId}, std::move(payload)};
}

} // namespace

class PlaybackShutdownTest final : public QObject
{
    Q_OBJECT

private slots:
    void commandBusRejectsAfterClose();
    void stopDuringLoading();
    void stopDuringPlaybackSuppressesLateSnapshots();
    void repeatedLifecycle100Cycles();
};

void PlaybackShutdownTest::commandBusRejectsAfterClose()
{
    PlaybackSession session;
    PlaybackCommandBus bus(session);

    QVERIFY(bus.isAcceptingCommands());
    bus.close();
    QVERIFY(!bus.isAcceptingCommands());

    QString error;
    QVERIFY(!bus.submit(
        makeCommand(1, TransportCommand{TransportAction::Play}),
        &error));
    QCOMPARE(error, QStringLiteral("Playback command bus is closed."));

    bus.close();
    QVERIFY(!bus.isAcceptingCommands());
}

void PlaybackShutdownTest::stopDuringLoading()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString mediaPath = directory.filePath(QStringLiteral("shutdown-loading.wav"));
    QString error;
    QVERIFY2(
        test_support::writeSilentPcmWav(mediaPath, 5000, &error),
        qPrintable(error));

    PlaybackSessionThread host;
    QSignalSpy readySpy(&host, &PlaybackSessionThread::ready);
    QSignalSpy stoppedSpy(&host, &PlaybackSessionThread::stopped);

    QVERIFY2(host.start(&error), qPrintable(error));
    QTRY_COMPARE_WITH_TIMEOUT(readySpy.count(), 1, 5000);
    QVERIFY(host.commandBus() != nullptr);
    QVERIFY2(
        host.commandBus()->submit(
            makeCommand(10, LoadMediaCommand{mediaPath}),
            &error),
        qPrintable(error));

    QVERIFY2(host.stop(&error), qPrintable(error));
    QVERIFY(!host.isRunning());
    QVERIFY(host.commandBus() == nullptr);
    QTRY_VERIFY_WITH_TIMEOUT(stoppedSpy.count() >= 1, 1000);

    QVERIFY2(host.stop(&error), qPrintable(error));
}

void PlaybackShutdownTest::stopDuringPlaybackSuppressesLateSnapshots()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString mediaPath = directory.filePath(QStringLiteral("shutdown-playing.wav"));
    QString error;
    QVERIFY2(
        test_support::writeSilentPcmWav(mediaPath, 30000, &error),
        qPrintable(error));

    PlaybackSessionThread host;
    StatePublisher* publisher = host.statePublisher();
    QVERIFY(publisher != nullptr);

    bool readySnapshotSeen = false;
    bool pausedSnapshotSeen = false;
    bool playingSnapshotSeen = false;
    bool closingSnapshotSeen = false;
    int publishedSnapshotCount = 0;
    QObject::connect(
        publisher,
        &StatePublisher::snapshotPublished,
        &host,
        [&](const PlaybackSnapshot& snapshot) {
            ++publishedSnapshotCount;
            if (snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.media().source.has_value()
                && *snapshot.media().source == mediaPath) {
                readySnapshotSeen = true;
            }
            if (snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Paused) {
                pausedSnapshotSeen = true;
            }
            if (snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Playing) {
                playingSnapshotSeen = true;
            }
            if (snapshot.lifecycle() == PlaybackLifecycleState::Closing) {
                closingSnapshotSeen = true;
            }
        });

    QSignalSpy readySpy(&host, &PlaybackSessionThread::ready);
    QVERIFY2(host.start(&error), qPrintable(error));
    QTRY_COMPARE_WITH_TIMEOUT(readySpy.count(), 1, 5000);
    QVERIFY(host.commandBus() != nullptr);

    QVERIFY2(
        host.commandBus()->submit(
            makeCommand(20, LoadMediaCommand{mediaPath}),
            &error),
        qPrintable(error));
    QTRY_VERIFY_WITH_TIMEOUT(readySnapshotSeen, 7000);

    // Force a confirmed transport transition before shutdown. A redundant Play while
    // libmpv is already unpaused is not required to emit another pause=false update.
    QVERIFY2(
        host.commandBus()->submit(
            makeCommand(21, TransportCommand{TransportAction::Pause}),
            &error),
        qPrintable(error));
    QTRY_VERIFY_WITH_TIMEOUT(pausedSnapshotSeen, 5000);

    QVERIFY2(
        host.commandBus()->submit(
            makeCommand(22, TransportCommand{TransportAction::Play}),
            &error),
        qPrintable(error));
    QTRY_VERIFY_WITH_TIMEOUT(playingSnapshotSeen, 5000);

    QVERIFY2(host.stop(&error), qPrintable(error));
    QVERIFY(!host.isRunning());
    QTRY_VERIFY_WITH_TIMEOUT(closingSnapshotSeen, 1000);

    const int countAfterClosing = publishedSnapshotCount;
    QTest::qWait(150);
    QCOMPARE(publishedSnapshotCount, countAfterClosing);
}

void PlaybackShutdownTest::repeatedLifecycle100Cycles()
{
    PlaybackSessionThread host;
    QString error;

    for (int cycle = 0; cycle < 100; ++cycle) {
        QSignalSpy readySpy(&host, &PlaybackSessionThread::ready);
        QSignalSpy startupFailureSpy(&host, &PlaybackSessionThread::startupFailed);

        QVERIFY2(
            host.start(&error),
            qPrintable(QStringLiteral("cycle %1 start failed: %2").arg(cycle).arg(error)));
        QTRY_COMPARE_WITH_TIMEOUT(readySpy.count(), 1, 5000);
        QCOMPARE(startupFailureSpy.count(), 0);
        QVERIFY(host.isRunning());
        QVERIFY(host.commandBus() != nullptr);
        QVERIFY(host.commandBus()->isAcceptingCommands());

        QVERIFY2(
            host.stop(&error),
            qPrintable(QStringLiteral("cycle %1 stop failed: %2").arg(cycle).arg(error)));
        QVERIFY(!host.isRunning());
        QVERIFY(host.commandBus() == nullptr);

        QCoreApplication::processEvents();
    }
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::PlaybackShutdownTest)
#include "playback_shutdown_test.moc"
