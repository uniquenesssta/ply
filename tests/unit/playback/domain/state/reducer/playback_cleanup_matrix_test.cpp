#include "playback/domain/state/playback_invariants.h"
#include "playback/domain/state/playback_reducer.h"

#include <QtTest/QTest>

#include <utility>

namespace player::playback::domain {
namespace {

PlaybackSnapshotState fullyPopulatedState()
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{31};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.transport = PlaybackTransportState::Playing;
    state.media.source = QStringLiteral("a.mkv");
    state.media.title = QStringLiteral("Media A");
    state.media.path = QStringLiteral("resolved-a.mkv");
    state.timeline.positionSeconds = 42.0;
    state.timeline.durationSeconds = 120.0;
    state.timeline.seekable = true;
    state.timeline.seeking = true;
    state.buffering.active = true;
    state.buffering.progressPercent = 64.0;

    CacheStatus cache;
    cache.durationSeconds = 8.0;
    state.buffering.cache = cache;

    state.controls.volumePercent = 73.0;
    state.controls.muted = false;
    state.controls.speed = 1.25;

    TrackDescriptor video;
    video.id = 1;
    video.kind = TrackKind::Video;
    video.selected = true;
    TrackDescriptor audio;
    audio.id = 2;
    audio.kind = TrackKind::Audio;
    audio.selected = true;
    TrackDescriptor subtitle;
    subtitle.id = 3;
    subtitle.kind = TrackKind::Subtitle;
    subtitle.selected = true;
    state.tracks.tracks = {video, audio, subtitle};
    state.tracks.selectedVideoId = 1;
    state.tracks.selectedAudioId = 2;
    state.tracks.selectedSubtitleId = 3;

    state.capabilities.hasVideoTrack = true;
    state.capabilities.hasAudioTrack = true;
    state.capabilities.hasSubtitleTrack = true;
    state.capabilities.hasChapters = true;

    ChapterDescriptor chapter;
    chapter.index = 0;
    chapter.startSeconds = 12.0;
    chapter.title = QStringLiteral("Chapter");
    state.chapters.chapters = {chapter};

    VideoStreamInfo videoInfo;
    videoInfo.width = 1920;
    videoInfo.height = 1080;
    state.streams.video = videoInfo;

    AudioStreamInfo audioInfo;
    audioInfo.sampleRate = 48000;
    audioInfo.channelCount = 2;
    state.streams.audio = audioInfo;

    state.failure = PlaybackFailure{
        PlaybackFailureCategory::Protocol,
        -1,
        QStringLiteral("stale diagnostic")};
    return state;
}

PlaybackEvent makePlaybackEvent(PlaybackEventPayload payload)
{
    return PlaybackEvent{std::move(payload)};
}

void verifySessionControlsPreserved(const PlaybackSnapshot& snapshot)
{
    QCOMPARE(*snapshot.controls().volumePercent, 73.0);
    QVERIFY(!*snapshot.controls().muted);
    QCOMPARE(*snapshot.controls().speed, 1.25);
}

void verifyMediaDetailsCleared(const PlaybackSnapshot& snapshot)
{
    QVERIFY(!snapshot.media().title.has_value());
    QVERIFY(!snapshot.media().path.has_value());
    QVERIFY(!snapshot.timeline().positionSeconds.has_value());
    QVERIFY(!snapshot.timeline().durationSeconds.has_value());
    QVERIFY(!snapshot.timeline().seekable.has_value());
    QVERIFY(!snapshot.timeline().seeking.has_value());
    QVERIFY(!snapshot.buffering().active);
    QVERIFY(!snapshot.buffering().progressPercent.has_value());
    QVERIFY(!snapshot.buffering().cache.has_value());
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
}

} // namespace

class PlaybackCleanupMatrixTest final : public QObject
{
    Q_OBJECT

private slots:
    void emptyToOpeningClearsResidualMediaState();
    void loadedAToOpeningBReplacesAllMediaScopedState();
    void stopClearsAllMediaScopedState();
    void failureKeepsOnlyDiagnosticMediaIdentity();
    void eofEndsAtKnownDurationAndPreservesDescription();
    void eofWithUnknownDurationPreservesLastKnownPosition();
};

void PlaybackCleanupMatrixTest::emptyToOpeningClearsResidualMediaState()
{
    PlaybackSnapshotState state = fullyPopulatedState();
    state.lifecycle = PlaybackLifecycleState::Empty;
    state.transport = PlaybackTransportState::Stopped;
    state.generation = MediaGeneration{41};
    state.media.source = QStringLiteral("a.mkv");

    const PlaybackSnapshot next = reducePlaybackSnapshot(
        PlaybackSnapshot{std::move(state)},
        makePlaybackEvent(MediaLoadStartedEvent{}));

    QCOMPARE(next.generation().value(), quint64{41});
    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Opening);
    QVERIFY(next.transport() == PlaybackTransportState::Idle);
    QCOMPARE(*next.media().source, QStringLiteral("a.mkv"));
    verifyMediaDetailsCleared(next);
    verifySessionControlsPreserved(next);
    QVERIFY(!next.failure().has_value());
    QVERIFY(checkPlaybackSnapshotInvariants(next).empty());
}

void PlaybackCleanupMatrixTest::loadedAToOpeningBReplacesAllMediaScopedState()
{
    const PlaybackSnapshot previous{fullyPopulatedState()};
    PlaybackSnapshotState seeded = previous.state();
    seeded.generation = MediaGeneration{32};
    seeded.media.source = QStringLiteral("b.mkv");

    const PlaybackSnapshot next = reducePlaybackSnapshot(
        PlaybackSnapshot{std::move(seeded)},
        makePlaybackEvent(MediaLoadStartedEvent{}));

    QCOMPARE(next.generation().value(), quint64{32});
    QCOMPARE(*next.media().source, QStringLiteral("b.mkv"));
    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Opening);
    QVERIFY(next.transport() == PlaybackTransportState::Idle);
    verifyMediaDetailsCleared(next);
    verifySessionControlsPreserved(next);
    QVERIFY(!next.failure().has_value());
    QVERIFY(checkPlaybackSnapshotInvariants(next).empty());
    QVERIFY(checkPlaybackTransitionInvariants(previous, next).empty());
}

void PlaybackCleanupMatrixTest::stopClearsAllMediaScopedState()
{
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        PlaybackSnapshot{fullyPopulatedState()},
        makePlaybackEvent(MediaEndedEvent{MediaEndReason::Stopped}));

    QCOMPARE(next.generation().value(), quint64{31});
    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Empty);
    QVERIFY(next.transport() == PlaybackTransportState::Stopped);
    QVERIFY(!next.media().source.has_value());
    verifyMediaDetailsCleared(next);
    verifySessionControlsPreserved(next);
    QVERIFY(!next.failure().has_value());
    QVERIFY(checkPlaybackSnapshotInvariants(next).empty());
}

void PlaybackCleanupMatrixTest::failureKeepsOnlyDiagnosticMediaIdentity()
{
    const PlaybackFailure failure{
        PlaybackFailureCategory::Media,
        -13,
        QStringLiteral("loading failed")};
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        PlaybackSnapshot{fullyPopulatedState()},
        makePlaybackEvent(MediaFailedEvent{failure}));

    QCOMPARE(next.generation().value(), quint64{31});
    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Failed);
    QVERIFY(next.transport() == PlaybackTransportState::Stopped);
    QCOMPARE(*next.media().source, QStringLiteral("a.mkv"));
    verifyMediaDetailsCleared(next);
    verifySessionControlsPreserved(next);
    QVERIFY(next.failure().has_value());
    QVERIFY(next.failure()->category == PlaybackFailureCategory::Media);
    QCOMPARE(next.failure()->backendCode, -13);
    QCOMPARE(next.failure()->diagnostic, QStringLiteral("loading failed"));
    QVERIFY(checkPlaybackSnapshotInvariants(next).empty());
}

void PlaybackCleanupMatrixTest::eofEndsAtKnownDurationAndPreservesDescription()
{
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        PlaybackSnapshot{fullyPopulatedState()},
        makePlaybackEvent(MediaEndedEvent{MediaEndReason::Eof}));

    QCOMPARE(next.generation().value(), quint64{31});
    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Ended);
    QVERIFY(next.transport() == PlaybackTransportState::Stopped);
    QCOMPARE(*next.media().source, QStringLiteral("a.mkv"));
    QCOMPARE(*next.media().title, QStringLiteral("Media A"));
    QCOMPARE(*next.media().path, QStringLiteral("resolved-a.mkv"));
    QCOMPARE(*next.timeline().positionSeconds, 120.0);
    QCOMPARE(*next.timeline().durationSeconds, 120.0);
    QVERIFY(*next.timeline().seekable);
    QVERIFY(next.timeline().seeking.has_value());
    QVERIFY(!*next.timeline().seeking);
    QVERIFY(!next.buffering().active);
    QVERIFY(!next.buffering().progressPercent.has_value());
    QVERIFY(!next.buffering().cache.has_value());
    QCOMPARE(next.tracks().tracks.size(), qsizetype{3});
    QCOMPARE(*next.tracks().selectedVideoId, qint64{1});
    QCOMPARE(*next.tracks().selectedAudioId, qint64{2});
    QCOMPARE(*next.tracks().selectedSubtitleId, qint64{3});
    QCOMPARE(next.chapters().chapters.size(), qsizetype{1});
    QVERIFY(next.streams().video.has_value());
    QVERIFY(next.streams().audio.has_value());
    QVERIFY(next.capabilities().hasVideoTrack);
    QVERIFY(next.capabilities().hasAudioTrack);
    QVERIFY(next.capabilities().hasSubtitleTrack);
    QVERIFY(next.capabilities().hasChapters);
    verifySessionControlsPreserved(next);
    QVERIFY(!next.failure().has_value());
    QVERIFY(checkPlaybackSnapshotInvariants(next).empty());
}

void PlaybackCleanupMatrixTest::eofWithUnknownDurationPreservesLastKnownPosition()
{
    PlaybackSnapshotState state = fullyPopulatedState();
    state.timeline.durationSeconds.reset();

    const PlaybackSnapshot next = reducePlaybackSnapshot(
        PlaybackSnapshot{std::move(state)},
        makePlaybackEvent(MediaEndedEvent{MediaEndReason::Eof}));

    QCOMPARE(*next.timeline().positionSeconds, 42.0);
    QVERIFY(!next.timeline().durationSeconds.has_value());
    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Ended);
    QVERIFY(checkPlaybackSnapshotInvariants(next).empty());
}

} // namespace player::playback::domain

QTEST_GUILESS_MAIN(player::playback::domain::PlaybackCleanupMatrixTest)
#include "playback_cleanup_matrix_test.moc"
