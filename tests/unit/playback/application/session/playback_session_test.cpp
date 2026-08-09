#include "fixtures/session_test_media.h"
#include "support/playback_session_test_harness.h"

#include "foundation/ids/request_id.h"
#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/session/playback_session_thread.h"
#include "playback/application/state_publisher/state_publisher.h"
#include "playback/domain/commands/playback_command.h"

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <QTemporaryDir>
#include <QThread>

#include <cmath>
#include <utility>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;

PlaybackCommand makeCommand(quint64 requestId, PlaybackCommandPayload payload)
{
    return PlaybackCommand{player::ids::RequestId{requestId}, std::move(payload)};
}

} // namespace

class PlaybackSessionTest final : public QObject
{
    Q_OBJECT

private slots:
    void realMediaMainPathRunsOnPlaybackThread();
    void threadHostStartsAndStops();
    void threadHostPublishesSnapshotsOnConsumerThread();
};

void PlaybackSessionTest::realMediaMainPathRunsOnPlaybackThread()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString mediaPath = directory.filePath(QStringLiteral("session-main-path.wav"));
    QString error;
    QVERIFY2(
        test_support::writeSilentPcmWav(mediaPath, 3000, &error),
        qPrintable(error));

    test_support::PlaybackSessionTestHarness harness;
    QVERIFY2(harness.start(&error), qPrintable(error));
    QCOMPARE(harness.readyThread(), harness.playbackThread());

    quint64 requestId = 1;

    quint64 sequence = harness.snapshotSequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, LoadMediaCommand{mediaPath}),
            &error),
        qPrintable(error));
    QVERIFY(harness.waitForSnapshotAfter(
        sequence,
        [&mediaPath](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.generation().value() == 1
                && snapshot.media().source.has_value()
                && *snapshot.media().source == mediaPath;
        },
        7000));

    sequence = harness.snapshotSequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, TransportCommand{TransportAction::Pause}),
            &error),
        qPrintable(error));
    QVERIFY(harness.waitForSnapshotAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Paused;
        },
        5000));

    sequence = harness.snapshotSequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, TransportCommand{TransportAction::Play}),
            &error),
        qPrintable(error));
    QVERIFY(harness.waitForSnapshotAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Playing;
        },
        5000));

    sequence = harness.snapshotSequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, TransportCommand{TransportAction::Pause}),
            &error),
        qPrintable(error));
    QVERIFY(harness.waitForSnapshotAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Paused;
        },
        5000));

    sequence = harness.snapshotSequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, SeekCommand{0.75, SeekMode::Absolute}),
            &error),
        qPrintable(error));
    QVERIFY(harness.waitForSnapshotAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.timeline().positionSeconds.has_value()
                && std::abs(*snapshot.timeline().positionSeconds - 0.75) < 0.35;
        },
        5000));

    sequence = harness.snapshotSequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, TransportCommand{TransportAction::Stop}),
            &error),
        qPrintable(error));
    QVERIFY(harness.waitForSnapshotAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Empty
                && snapshot.transport() == PlaybackTransportState::Stopped
                && snapshot.generation().value() == 1
                && !snapshot.media().source.has_value();
        },
        5000));

    QCOMPARE(harness.snapshotCallbackThread(), harness.playbackThread());
    QCOMPARE(harness.invariantViolations(), 0);
    QVERIFY2(harness.stop(&error), qPrintable(error));
}

void PlaybackSessionTest::threadHostStartsAndStops()
{
    PlaybackSessionThread host;
    QSignalSpy readySpy(&host, &PlaybackSessionThread::ready);
    QSignalSpy startupFailureSpy(&host, &PlaybackSessionThread::startupFailed);
    QSignalSpy stoppedSpy(&host, &PlaybackSessionThread::stopped);

    QVERIFY(host.statePublisher() != nullptr);
    QCOMPARE(host.statePublisher()->thread(), QThread::currentThread());

    QString error;
    QVERIFY2(host.start(&error), qPrintable(error));
    QTRY_COMPARE_WITH_TIMEOUT(readySpy.count(), 1, 5000);
    QCOMPARE(startupFailureSpy.count(), 0);
    QVERIFY(host.isRunning());
    QVERIFY(host.commandBus() != nullptr);

    QVERIFY2(host.stop(&error), qPrintable(error));
    QVERIFY(!host.isRunning());
    QTRY_VERIFY_WITH_TIMEOUT(stoppedSpy.count() >= 1, 1000);
}

void PlaybackSessionTest::threadHostPublishesSnapshotsOnConsumerThread()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString mediaPath = directory.filePath(QStringLiteral("publisher-host-path.wav"));
    QString error;
    QVERIFY2(
        test_support::writeSilentPcmWav(mediaPath, 1000, &error),
        qPrintable(error));

    PlaybackSessionThread host;
    StatePublisher* publisher = host.statePublisher();
    QVERIFY(publisher != nullptr);

    bool readyPublished = false;
    QThread* publishThread = nullptr;
    QObject::connect(
        publisher,
        &StatePublisher::snapshotPublished,
        &host,
        [&readyPublished, &publishThread, &mediaPath](const PlaybackSnapshot& snapshot) {
            publishThread = QThread::currentThread();
            if (snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.media().source.has_value()
                && *snapshot.media().source == mediaPath) {
                readyPublished = true;
            }
        });

    QSignalSpy readySpy(&host, &PlaybackSessionThread::ready);
    QVERIFY2(host.start(&error), qPrintable(error));
    QTRY_COMPARE_WITH_TIMEOUT(readySpy.count(), 1, 5000);
    QVERIFY(host.commandBus() != nullptr);
    QVERIFY2(
        host.commandBus()->submit(
            makeCommand(100, LoadMediaCommand{mediaPath}),
            &error),
        qPrintable(error));

    QTRY_VERIFY_WITH_TIMEOUT(readyPublished, 7000);
    QCOMPARE(publishThread, QThread::currentThread());
    QCOMPARE(publisher->thread(), QThread::currentThread());

    QVERIFY2(host.stop(&error), qPrintable(error));
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::PlaybackSessionTest)
#include "playback_session_test.moc"
