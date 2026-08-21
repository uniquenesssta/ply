#include "playback/domain/state/playback_snapshot.h"

#include <QtTest/QTest>

#include <limits>
#include <optional>
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
    void chapterValidationBoundsRowsWhenDurationIsKnown();
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
    QVERIFY(!snapshot.buffering().cache.has_value());
    QVERIFY(!snapshot.controls().volumePercent.has_value());
    QVERIFY(!snapshot.controls().muted.has_value());
    QVERIFY(!snapshot.controls().speed.has_value());
    QVERIFY(!snapshot.capabilities().hasVideoTrack);
    QVERIFY(!snapshot.capabilities().hasAudioTrack);
    QVERIFY(!snapshot.capabilities().hasSubtitleTrack);
    QVERIFY(!snapshot.capabilities().hasChapters);
    QVERIFY(!snapshot.streams().video.has_value());
    QVERIFY(!snapshot.streams().audio.has_value());
    QVERIFY(snapshot.tracks().tracks.isEmpty());
    QVERIFY(!snapshot.tracks().selectedVideoId.has_value());
    QVERIFY(!snapshot.tracks().selectedAudioId.has_value());
    QVERIFY(!snapshot.tracks().selectedSubtitleId.has_value());
    QVERIFY(snapshot.chapters().chapters.isEmpty());
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
    QVERIFY(snapshot.tracks().tracks.isEmpty());
    QVERIFY(snapshot.chapters().chapters.isEmpty());
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
    QVERIFY(!snapshot.buffering().cache.has_value());
    QVERIFY(snapshot.tracks().tracks.isEmpty());
    QVERIFY(snapshot.chapters().chapters.isEmpty());
    QVERIFY(!snapshot.streams().video.has_value());
    QVERIFY(!snapshot.streams().audio.has_value());
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
    CacheStatus cache;
    cache.durationSeconds = 4.0;
    cache.forwardBytes = 4096;
    state.buffering.cache = cache;
    state.controls.volumePercent = 75.0;
    state.controls.muted = false;
    state.controls.speed = 1.25;

    TrackDescriptor audioTrack;
    audioTrack.id = 2;
    audioTrack.kind = TrackKind::Audio;
    audioTrack.title = QStringLiteral("Main Audio");
    audioTrack.selected = true;
    state.tracks.tracks.append(audioTrack);
    state.tracks.selectedAudioId = 2;

    ChapterDescriptor chapter;
    chapter.index = 0;
    chapter.startSeconds = 10.0;
    chapter.title = QStringLiteral("Chapter One");
    state.chapters.chapters.append(chapter);

    AudioStreamInfo audioInfo;
    audioInfo.sampleRate = 48000;
    audioInfo.channelCount = 2;
    state.streams.audio = audioInfo;
    state.capabilities.hasAudioTrack = true;
    state.capabilities.hasChapters = true;

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
    QVERIFY(snapshot.buffering().cache.has_value());
    QCOMPARE(*snapshot.buffering().cache->forwardBytes, qint64{4096});
    QCOMPARE(*snapshot.controls().volumePercent, 75.0);
    QVERIFY(!*snapshot.controls().muted);
    QCOMPARE(*snapshot.controls().speed, 1.25);
    QVERIFY(snapshot.capabilities().hasAudioTrack);
    QVERIFY(snapshot.capabilities().hasChapters);
    QCOMPARE(snapshot.tracks().tracks.size(), qsizetype{1});
    QCOMPARE(*snapshot.tracks().selectedAudioId, qint64{2});
    QCOMPARE(snapshot.chapters().chapters.size(), qsizetype{1});
    QVERIFY(snapshot.streams().audio.has_value());
    QCOMPARE(*snapshot.streams().audio->sampleRate, qint64{48000});
    QVERIFY(!snapshot.failure().has_value());
}

void PlaybackSnapshotTest::chapterValidationBoundsRowsWhenDurationIsKnown()
{
    PlaybackChapterState state;
    state.chapters = {
        ChapterDescriptor{0, 0.0, QStringLiteral("Opening")},
        ChapterDescriptor{1, 90.0, QStringLiteral("At Duration")},
        ChapterDescriptor{2, 90.01, QStringLiteral("Past Duration")},
        ChapterDescriptor{3, -1.0, QStringLiteral("Negative")},
        ChapterDescriptor{-1, 1.0, QStringLiteral("Invalid Index")},
        ChapterDescriptor{
            4,
            std::numeric_limits<double>::infinity(),
            QStringLiteral("Infinite")},
    };

    const auto unbounded = state.validatedForDuration(std::nullopt);
    QCOMPARE(unbounded.size(), 3);
    QCOMPARE(unbounded.at(0).index, qsizetype{0});
    QCOMPARE(unbounded.at(1).index, qsizetype{1});
    QCOMPARE(unbounded.at(2).index, qsizetype{2});

    const auto bounded = state.validatedForDuration(90.0);
    QCOMPARE(bounded.size(), 2);
    QCOMPARE(bounded.at(0).index, qsizetype{0});
    QCOMPARE(bounded.at(1).index, qsizetype{1});
    QCOMPARE(bounded.at(1).startSeconds, 90.0);
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
