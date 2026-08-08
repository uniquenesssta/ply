#include "playback/domain/state/playback_snapshot.h"

#include <QtTest/QTest>

#include <utility>

namespace player::playback::domain {

class PlaybackSnapshotTest final : public QObject
{
    Q_OBJECT

private slots:
    void defaultSnapshotIsEmpty();
    void openingSnapshotOwnsMediaIdentity();
    void stoppedSnapshotClearsMediaScopedState();
    void explicitStatePreservesIndependentAxes();
    void failureStateCarriesTypedDiagnostic();
};

void PlaybackSnapshotTest::defaultSnapshotIsEmpty()
{
    const PlaybackSnapshot snapshot;

    QVERIFY(!snapshot.generation().isValid());
    QVERIFY(snapshot.lifecycle() == PlaybackLifecycleState::Empty);
    QVERIFY(snapshot.transport() == PlaybackTransportState::Idle);
    QVERIFY(!snapshot.media().source.has_value());
    QVERIFY(!snapshot.media().title.has_value());
    QVERIFY(!snapshot.media().path.has_value());
    QVERIFY(!snapshot.timeline().positionSeconds.has_value());
    QVERIFY(!snapshot.timeline().durationSeconds.has_value());
    QVERIFY(!snapshot.timeline().seekable.has_value());
    QVERIFY(!snapshot.timeline().seeking.has_value());
    QVERIFY(!snapshot.buffering().active);
    QVERIFY(!snapshot.buffering().progressPercent.has_value());
    QVERIFY(!snapshot.controls().volumePercent.has_value());
    QVERIFY(!snapshot.controls().muted.has_value());
    QVERIFY(!snapshot.controls().speed.has_value());
    QVERIFY(!snapshot.failure().has_value());
}

void PlaybackSnapshotTest::openingSnapshotOwnsMediaIdentity()
{
    const PlaybackSnapshot snapshot = PlaybackSnapshot::opening(
        MediaGeneration{7},
        QStringLiteral("sample.mp4"));

    QCOMPARE(snapshot.generation().value(), quint64{7});
    QVERIFY(snapshot.lifecycle() == PlaybackLifecycleState::Opening);
    QVERIFY(snapshot.transport() == PlaybackTransportState::Idle);
    QVERIFY(snapshot.media().source.has_value());
    QCOMPARE(*snapshot.media().source, QStringLiteral("sample.mp4"));
    QVERIFY(!snapshot.media().title.has_value());
    QVERIFY(!snapshot.media().path.has_value());
    QVERIFY(!snapshot.timeline().positionSeconds.has_value());
    QVERIFY(!snapshot.timeline().durationSeconds.has_value());
    QVERIFY(!snapshot.buffering().active);
    QVERIFY(!snapshot.failure().has_value());
}

void PlaybackSnapshotTest::stoppedSnapshotClearsMediaScopedState()
{
    const PlaybackSnapshot snapshot = PlaybackSnapshot::stopped(MediaGeneration{11});

    QCOMPARE(snapshot.generation().value(), quint64{11});
    QVERIFY(snapshot.lifecycle() == PlaybackLifecycleState::Empty);
    QVERIFY(snapshot.transport() == PlaybackTransportState::Stopped);
    QVERIFY(!snapshot.media().source.has_value());
    QVERIFY(!snapshot.media().title.has_value());
    QVERIFY(!snapshot.media().path.has_value());
    QVERIFY(!snapshot.timeline().positionSeconds.has_value());
    QVERIFY(!snapshot.timeline().durationSeconds.has_value());
    QVERIFY(!snapshot.timeline().seekable.has_value());
    QVERIFY(!snapshot.timeline().seeking.has_value());
    QVERIFY(!snapshot.buffering().active);
    QVERIFY(!snapshot.buffering().progressPercent.has_value());
    QVERIFY(!snapshot.failure().has_value());
}

void PlaybackSnapshotTest::explicitStatePreservesIndependentAxes()
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{19};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.transport = PlaybackTransportState::Paused;
    state.media.source = QStringLiteral("https://example.test/media");
    state.media.title = QStringLiteral("Example");
    state.media.path = QStringLiteral("https://example.test/media");
    state.timeline.positionSeconds = 12.5;
    state.timeline.durationSeconds = 90.0;
    state.timeline.seekable = true;
    state.timeline.seeking = false;
    state.buffering.active = true;
    state.buffering.progressPercent = 42.0;
    state.controls.volumePercent = 75.0;
    state.controls.muted = false;
    state.controls.speed = 1.25;

    const PlaybackSnapshot snapshot{std::move(state)};

    QCOMPARE(snapshot.generation().value(), quint64{19});
    QVERIFY(snapshot.lifecycle() == PlaybackLifecycleState::Ready);
    QVERIFY(snapshot.transport() == PlaybackTransportState::Paused);
    QCOMPARE(*snapshot.media().title, QStringLiteral("Example"));
    QCOMPARE(*snapshot.timeline().positionSeconds, 12.5);
    QCOMPARE(*snapshot.timeline().durationSeconds, 90.0);
    QVERIFY(*snapshot.timeline().seekable);
    QVERIFY(!*snapshot.timeline().seeking);
    QVERIFY(snapshot.buffering().active);
    QCOMPARE(*snapshot.buffering().progressPercent, 42.0);
    QCOMPARE(*snapshot.controls().volumePercent, 75.0);
    QVERIFY(!*snapshot.controls().muted);
    QCOMPARE(*snapshot.controls().speed, 1.25);
    QVERIFY(!snapshot.failure().has_value());
}

void PlaybackSnapshotTest::failureStateCarriesTypedDiagnostic()
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{23};
    state.lifecycle = PlaybackLifecycleState::Failed;
    state.transport = PlaybackTransportState::Stopped;
    state.media.source = QStringLiteral("broken-file.mp4");
    state.failure = PlaybackFailure{
        PlaybackFailureCategory::Media,
        -13,
        QStringLiteral("loading failed")};

    const PlaybackSnapshot snapshot{std::move(state)};

    QVERIFY(snapshot.failure().has_value());
    QVERIFY(snapshot.failure()->category == PlaybackFailureCategory::Media);
    QCOMPARE(snapshot.failure()->backendCode, -13);
    QCOMPARE(snapshot.failure()->diagnostic, QStringLiteral("loading failed"));
}

} // namespace player::playback::domain

QTEST_GUILESS_MAIN(player::playback::domain::PlaybackSnapshotTest)
#include "playback_snapshot_test.moc"
