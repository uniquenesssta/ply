#include "fixtures/session_test_media.h"
#include "support/playback_session_test_harness.h"

#include "foundation/ids/request_id.h"
#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/transport_command.h"

#include <QtTest/QTest>

#include <QTemporaryDir>

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

class PlaybackEndedSeekTest final : public QObject
{
    Q_OBJECT

private slots:
    void endedMediaCanSeekEarlierWithoutReload();
};

void PlaybackEndedSeekTest::endedMediaCanSeekEarlierWithoutReload()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString mediaPath = directory.filePath(QStringLiteral("ended-seek.wav"));
    QString error;
    QVERIFY2(
        test_support::writeSilentPcmWav(mediaPath, 3000, &error),
        qPrintable(error));

    test_support::PlaybackSessionTestHarness harness;
    QVERIFY2(harness.start(&error), qPrintable(error));

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
                && *snapshot.media().source == mediaPath
                && snapshot.timeline().durationSeconds.has_value()
                && snapshot.timeline().seekable.value_or(false);
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
    QVERIFY(harness.waitForSnapshotAfter(
        sequence,
        [&mediaPath](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ended
                && snapshot.transport() == PlaybackTransportState::Stopped
                && snapshot.generation().value() == 1
                && snapshot.media().source.has_value()
                && *snapshot.media().source == mediaPath
                && snapshot.timeline().durationSeconds.has_value()
                && snapshot.timeline().seekable.value_or(false);
        },
        7000));

    sequence = harness.snapshotSequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, SeekCommand{0.25, SeekMode::Absolute}),
            &error),
        qPrintable(error));
    QVERIFY(harness.waitForSnapshotAfter(
        sequence,
        [&mediaPath](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Paused
                && snapshot.generation().value() == 1
                && snapshot.media().source.has_value()
                && *snapshot.media().source == mediaPath
                && snapshot.timeline().positionSeconds.has_value()
                && std::abs(*snapshot.timeline().positionSeconds - 0.25) < 0.35;
        },
        5000));

    QCOMPARE(harness.invariantViolations(), 0);
    QVERIFY2(harness.stop(&error), qPrintable(error));
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::PlaybackEndedSeekTest)
#include "playback_ended_seek_test.moc"
