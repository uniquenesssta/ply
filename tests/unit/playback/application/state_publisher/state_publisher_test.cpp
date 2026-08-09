#include "playback/application/state_publisher/state_publisher.h"

#include "playback/domain/errors/playback_failure.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <QMetaObject>
#include <QThread>

#include <utility>

namespace player::playback::application {

class SnapshotProducer final : public QObject
{
    Q_OBJECT

public slots:
    void publish(player::playback::domain::PlaybackSnapshot snapshot)
    {
        emit snapshotProduced(snapshot);
    }

signals:
    void snapshotProduced(const player::playback::domain::PlaybackSnapshot& snapshot);
};

namespace {

using namespace player::playback::domain;

PlaybackSnapshot readySnapshot(double positionSeconds = 0.0)
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{1};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.transport = PlaybackTransportState::Playing;
    state.media.source = QStringLiteral("publisher-test.wav");
    state.timeline.positionSeconds = positionSeconds;
    state.timeline.durationSeconds = 10.0;
    state.timeline.seekable = true;
    state.timeline.seeking = false;
    state.controls.volumePercent = 50.0;
    state.controls.muted = false;
    state.controls.speed = 1.0;
    return PlaybackSnapshot{std::move(state)};
}

PlaybackSnapshot withPosition(const PlaybackSnapshot& snapshot, double positionSeconds)
{
    PlaybackSnapshotState state = snapshot.state();
    state.timeline.positionSeconds = positionSeconds;
    return PlaybackSnapshot{std::move(state)};
}

PlaybackSnapshot withTransport(
    const PlaybackSnapshot& snapshot,
    PlaybackTransportState transport,
    double positionSeconds)
{
    PlaybackSnapshotState state = snapshot.state();
    state.transport = transport;
    state.timeline.positionSeconds = positionSeconds;
    return PlaybackSnapshot{std::move(state)};
}

PlaybackSnapshot withFailure(const PlaybackSnapshot& snapshot)
{
    PlaybackSnapshotState state = snapshot.state();
    state.failure = PlaybackFailure{
        PlaybackFailureCategory::Protocol,
        7,
        QStringLiteral("publisher-test-failure")};
    return PlaybackSnapshot{std::move(state)};
}

PlaybackSnapshot withAudioTrack(
    const PlaybackSnapshot& snapshot,
    double positionSeconds)
{
    PlaybackSnapshotState state = snapshot.state();
    state.timeline.positionSeconds = positionSeconds;
    TrackDescriptor audio;
    audio.id = 2;
    audio.kind = TrackKind::Audio;
    audio.selected = true;
    state.tracks.tracks = {audio};
    state.tracks.selectedAudioId = 2;
    state.capabilities.hasAudioTrack = true;
    return PlaybackSnapshot{std::move(state)};
}

PlaybackSnapshot spySnapshot(const QSignalSpy& spy, qsizetype index)
{
    return qvariant_cast<PlaybackSnapshot>(spy.at(index).at(0));
}

} // namespace

class StatePublisherTest final : public QObject
{
    Q_OBJECT

private slots:
    void firstSnapshotPublishesImmediately();
    void duplicateSnapshotIsSuppressed();
    void positionBurstIsCoalesced();
    void transportChangeBypassesPositionThrottle();
    void failureChangeBypassesPositionThrottle();
    void mediaAxisChangeBypassesPositionThrottle();
    void queuedProducerPublishesOnConsumerThread();
};

void StatePublisherTest::firstSnapshotPublishesImmediately()
{
    StatePublisher publisher;
    QSignalSpy spy(&publisher, &StatePublisher::snapshotPublished);

    publisher.acceptSnapshot(readySnapshot(0.0));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(*spySnapshot(spy, 0).timeline().positionSeconds, 0.0);
}

void StatePublisherTest::duplicateSnapshotIsSuppressed()
{
    StatePublisher publisher;
    QSignalSpy spy(&publisher, &StatePublisher::snapshotPublished);
    const PlaybackSnapshot snapshot = readySnapshot(1.0);

    publisher.acceptSnapshot(snapshot);
    publisher.acceptSnapshot(snapshot);

    QCOMPARE(spy.count(), 1);
}

void StatePublisherTest::positionBurstIsCoalesced()
{
    StatePublisher publisher;
    QSignalSpy spy(&publisher, &StatePublisher::snapshotPublished);
    const PlaybackSnapshot initial = readySnapshot(0.0);

    publisher.acceptSnapshot(initial);
    publisher.acceptSnapshot(withPosition(initial, 0.1));
    publisher.acceptSnapshot(withPosition(initial, 0.2));
    publisher.acceptSnapshot(withPosition(initial, 0.3));

    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 2, 250);
    QCOMPARE(*spySnapshot(spy, 1).timeline().positionSeconds, 0.3);

    QTest::qWait(StatePublisher::positionPublishIntervalMilliseconds() * 2);
    QCOMPARE(spy.count(), 2);
}

void StatePublisherTest::transportChangeBypassesPositionThrottle()
{
    StatePublisher publisher;
    QSignalSpy spy(&publisher, &StatePublisher::snapshotPublished);
    const PlaybackSnapshot initial = readySnapshot(0.0);

    publisher.acceptSnapshot(initial);
    publisher.acceptSnapshot(withPosition(initial, 0.1));
    publisher.acceptSnapshot(withTransport(initial, PlaybackTransportState::Paused, 0.2));

    QCOMPARE(spy.count(), 2);
    const PlaybackSnapshot published = spySnapshot(spy, 1);
    QCOMPARE(published.transport(), PlaybackTransportState::Paused);
    QCOMPARE(*published.timeline().positionSeconds, 0.2);

    QTest::qWait(StatePublisher::positionPublishIntervalMilliseconds() * 2);
    QCOMPARE(spy.count(), 2);
}

void StatePublisherTest::failureChangeBypassesPositionThrottle()
{
    StatePublisher publisher;
    QSignalSpy spy(&publisher, &StatePublisher::snapshotPublished);
    const PlaybackSnapshot initial = readySnapshot(0.0);

    publisher.acceptSnapshot(initial);
    publisher.acceptSnapshot(withPosition(initial, 0.1));
    publisher.acceptSnapshot(withFailure(withPosition(initial, 0.2)));

    QCOMPARE(spy.count(), 2);
    const PlaybackSnapshot published = spySnapshot(spy, 1);
    QVERIFY(published.failure().has_value());
    QCOMPARE(published.failure()->diagnostic, QStringLiteral("publisher-test-failure"));

    QTest::qWait(StatePublisher::positionPublishIntervalMilliseconds() * 2);
    QCOMPARE(spy.count(), 2);
}

void StatePublisherTest::mediaAxisChangeBypassesPositionThrottle()
{
    StatePublisher publisher;
    QSignalSpy spy(&publisher, &StatePublisher::snapshotPublished);
    const PlaybackSnapshot initial = readySnapshot(0.0);

    publisher.acceptSnapshot(initial);
    publisher.acceptSnapshot(withPosition(initial, 0.1));
    publisher.acceptSnapshot(withAudioTrack(initial, 0.2));

    QCOMPARE(spy.count(), 2);
    const PlaybackSnapshot published = spySnapshot(spy, 1);
    QVERIFY(published.capabilities().hasAudioTrack);
    QCOMPARE(published.tracks().tracks.size(), qsizetype{1});
    QCOMPARE(*published.tracks().selectedAudioId, qint64{2});
    QCOMPARE(*published.timeline().positionSeconds, 0.2);

    QTest::qWait(StatePublisher::positionPublishIntervalMilliseconds() * 2);
    QCOMPARE(spy.count(), 2);
}

void StatePublisherTest::queuedProducerPublishesOnConsumerThread()
{
    StatePublisher publisher;
    QSignalSpy spy(&publisher, &StatePublisher::snapshotPublished);
    QThread worker;
    worker.setObjectName(QStringLiteral("StatePublisherProducerThread"));

    auto* producer = new SnapshotProducer();
    producer->moveToThread(&worker);
    QObject::connect(&worker, &QThread::finished, producer, &QObject::deleteLater);
    QObject::connect(
        producer,
        &SnapshotProducer::snapshotProduced,
        &publisher,
        &StatePublisher::acceptSnapshot,
        Qt::QueuedConnection);

    QThread* callbackThread = nullptr;
    QObject::connect(
        &publisher,
        &StatePublisher::snapshotPublished,
        &publisher,
        [&callbackThread](const PlaybackSnapshot&) {
            callbackThread = QThread::currentThread();
        });

    worker.start();
    const PlaybackSnapshot snapshot = readySnapshot(2.0);
    QVERIFY(QMetaObject::invokeMethod(
        producer,
        [producer, snapshot]() {
            producer->publish(snapshot);
        },
        Qt::QueuedConnection));

    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 1000);
    QCOMPARE(callbackThread, QThread::currentThread());
    QCOMPARE(publisher.thread(), QThread::currentThread());

    worker.quit();
    QVERIFY(worker.wait(1000));
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::StatePublisherTest)
#include "state_publisher_test.moc"
