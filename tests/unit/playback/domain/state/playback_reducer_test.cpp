#include "foundation/ids/request_id.h"
#include "playback/domain/state/playback_invariants.h"
#include "playback/domain/state/playback_reducer.h"

#include <QtTest/QTest>

#include <optional>
#include <utility>

namespace player::playback::domain {
namespace {

PlaybackSnapshot populatedSnapshot()
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{17};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.transport = PlaybackTransportState::Playing;
    state.media.source = QStringLiteral("next.mp4");
    state.media.title = QStringLiteral("Old title");
    state.media.path = QStringLiteral("old-path.mp4");
    state.timeline.positionSeconds = 41.0;
    state.timeline.durationSeconds = 120.0;
    state.timeline.seekable = true;
    state.timeline.seeking = true;
    state.buffering.active = true;
    state.buffering.progressPercent = 63.0;
    CacheStatus cache;
    cache.durationSeconds = 3.0;
    state.buffering.cache = cache;
    state.controls.volumePercent = 72.0;
    state.controls.muted = false;
    state.controls.speed = 1.25;

    TrackDescriptor audioTrack;
    audioTrack.id = 2;
    audioTrack.kind = TrackKind::Audio;
    audioTrack.selected = true;
    state.tracks.tracks.append(audioTrack);
    state.tracks.selectedAudioId = 2;
    state.capabilities.hasAudioTrack = true;

    ChapterDescriptor chapter;
    chapter.index = 0;
    chapter.startSeconds = 10.0;
    state.chapters.chapters.append(chapter);
    state.capabilities.hasChapters = true;

    AudioStreamInfo audioInfo;
    audioInfo.sampleRate = 48000;
    state.streams.audio = audioInfo;

    state.failure = PlaybackFailure{
        PlaybackFailureCategory::Protocol,
        -1,
        QStringLiteral("old failure")};
    return PlaybackSnapshot{std::move(state)};
}

PlaybackEvent makePlaybackEvent(PlaybackEventPayload payload)
{
    return PlaybackEvent{std::move(payload)};
}

} // namespace

class PlaybackReducerTest final : public QObject
{
    Q_OBJECT

private slots:
    void loadStartedClearsMediaScopedState();
    void fileLoadedMarksMediaReady();
    void pauseAndBufferingStayIndependent();
    void propertyEventsUpdateTheirOwnAxes();
    void mediaAxesUpdateIndependently();
    void invalidTrackSelectionClearsOnlyItsKind();
    void unavailablePauseAndBufferingDoNotInventState();
    void eofMarksEndedWithoutDiscardingMediaIdentity();
    void eofPropertyMarksEndedAndSeekBackRecoversPaused();
    void stopClearsMediaScopedStateButPreservesControls();
    void redirectReopensAndDropsOldMediaDetails();
    void mediaFailureClearsStaleMediaStateAndKeepsSource();
    void backendShutdownMarksClosing();
    void requestAndObservationEventsDoNotOwnSnapshotState();
    void protocolFailureIsRecordedWithoutInventingMediaFailure();
};

void PlaybackReducerTest::loadStartedClearsMediaScopedState()
{
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(MediaLoadStartedEvent{}));

    QCOMPARE(next.generation().value(), quint64{17});
    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Opening);
    QVERIFY(next.transport() == PlaybackTransportState::Idle);
    QCOMPARE(*next.media().source, QStringLiteral("next.mp4"));
    QVERIFY(!next.media().title.has_value());
    QVERIFY(!next.media().path.has_value());
    QVERIFY(!next.timeline().positionSeconds.has_value());
    QVERIFY(!next.timeline().durationSeconds.has_value());
    QVERIFY(!next.timeline().seekable.has_value());
    QVERIFY(!next.timeline().seeking.has_value());
    QVERIFY(!next.buffering().active);
    QVERIFY(!next.buffering().progressPercent.has_value());
    QVERIFY(!next.buffering().cache.has_value());
    QVERIFY(next.tracks().tracks.isEmpty());
    QVERIFY(next.chapters().chapters.isEmpty());
    QVERIFY(!next.streams().audio.has_value());
    QVERIFY(!next.capabilities().hasAudioTrack);
    QVERIFY(!next.capabilities().hasChapters);
    QCOMPARE(*next.controls().volumePercent, 72.0);
    QCOMPARE(*next.controls().speed, 1.25);
    QVERIFY(!next.failure().has_value());
}

void PlaybackReducerTest::fileLoadedMarksMediaReady()
{
    const PlaybackSnapshot opening = PlaybackSnapshot::opening(
        MediaGeneration{3},
        QStringLiteral("sample.mp4"));
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        opening,
        makePlaybackEvent(MediaLoadedEvent{}));

    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Ready);
    QVERIFY(next.transport() == PlaybackTransportState::Idle);
    QCOMPARE(*next.media().source, QStringLiteral("sample.mp4"));
}

void PlaybackReducerTest::pauseAndBufferingStayIndependent()
{
    PlaybackSnapshot snapshot = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(PauseChangedEvent{true}));
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(BufferingChangedEvent{true}));
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(BufferingProgressChangedEvent{42.0}));

    QVERIFY(snapshot.transport() == PlaybackTransportState::Paused);
    QVERIFY(snapshot.buffering().active);
    QCOMPARE(*snapshot.buffering().progressPercent, 42.0);

    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(BufferingChangedEvent{false}));

    QVERIFY(snapshot.transport() == PlaybackTransportState::Paused);
    QVERIFY(!snapshot.buffering().active);
    QVERIFY(!snapshot.buffering().progressPercent.has_value());
    QVERIFY(snapshot.buffering().cache.has_value());
}

void PlaybackReducerTest::propertyEventsUpdateTheirOwnAxes()
{
    PlaybackSnapshot snapshot = PlaybackSnapshot::opening(
        MediaGeneration{5},
        QStringLiteral("sample.mp4"));

    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(PositionChangedEvent{12.5}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(DurationChangedEvent{90.0}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(SeekableChangedEvent{true}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(SeekingChangedEvent{false}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(MediaTitleChangedEvent{QStringLiteral("Title")}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(MediaPathChangedEvent{QStringLiteral("resolved.mp4")}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(VolumeChangedEvent{80.0}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(MuteChangedEvent{true}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(SpeedChangedEvent{1.5}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(PauseChangedEvent{false}));

    QCOMPARE(*snapshot.timeline().positionSeconds, 12.5);
    QCOMPARE(*snapshot.timeline().durationSeconds, 90.0);
    QVERIFY(*snapshot.timeline().seekable);
    QVERIFY(!*snapshot.timeline().seeking);
    QCOMPARE(*snapshot.media().title, QStringLiteral("Title"));
    QCOMPARE(*snapshot.media().path, QStringLiteral("resolved.mp4"));
    QCOMPARE(*snapshot.controls().volumePercent, 80.0);
    QVERIFY(*snapshot.controls().muted);
    QCOMPARE(*snapshot.controls().speed, 1.5);
    QVERIFY(snapshot.transport() == PlaybackTransportState::Playing);
}

void PlaybackReducerTest::mediaAxesUpdateIndependently()
{
    PlaybackSnapshot snapshot = PlaybackSnapshot::opening(
        MediaGeneration{8},
        QStringLiteral("sample.mkv"));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(MediaLoadedEvent{}));

    TrackDescriptor videoTrack;
    videoTrack.id = 1;
    videoTrack.kind = TrackKind::Video;
    videoTrack.selected = true;
    TrackDescriptor audioTrack;
    audioTrack.id = 2;
    audioTrack.kind = TrackKind::Audio;
    audioTrack.selected = true;
    TrackDescriptor subtitleTrack;
    subtitleTrack.id = 3;
    subtitleTrack.kind = TrackKind::Subtitle;

    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(TrackListChangedEvent{{videoTrack, audioTrack, subtitleTrack}}));
    QVERIFY(snapshot.capabilities().hasVideoTrack);
    QVERIFY(snapshot.capabilities().hasAudioTrack);
    QVERIFY(snapshot.capabilities().hasSubtitleTrack);
    QCOMPARE(*snapshot.tracks().selectedVideoId, qint64{1});
    QCOMPARE(*snapshot.tracks().selectedAudioId, qint64{2});
    QVERIFY(!snapshot.tracks().selectedSubtitleId.has_value());

    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(SelectedSubtitleTrackChangedEvent{qint64{3}}));
    QCOMPARE(*snapshot.tracks().selectedSubtitleId, qint64{3});

    ChapterDescriptor chapter;
    chapter.index = 0;
    chapter.startSeconds = 15.0;
    chapter.title = QStringLiteral("Scene");
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(ChapterListChangedEvent{{chapter}}));
    QVERIFY(snapshot.capabilities().hasChapters);
    QCOMPARE(snapshot.chapters().chapters.size(), qsizetype{1});

    VideoStreamInfo videoInfo;
    videoInfo.width = 1920;
    videoInfo.height = 1080;
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(VideoStreamInfoChangedEvent{videoInfo}));
    AudioStreamInfo audioInfo;
    audioInfo.sampleRate = 48000;
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(AudioStreamInfoChangedEvent{audioInfo}));
    QVERIFY(snapshot.streams().video.has_value());
    QVERIFY(snapshot.streams().audio.has_value());
    QCOMPARE(*snapshot.streams().video->width, qint64{1920});
    QCOMPARE(*snapshot.streams().audio->sampleRate, qint64{48000});

    CacheStatus cache;
    cache.durationSeconds = 5.5;
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(CacheStatusChangedEvent{cache}));
    QVERIFY(snapshot.buffering().cache.has_value());
    QCOMPARE(*snapshot.buffering().cache->durationSeconds, 5.5);
}

void PlaybackReducerTest::invalidTrackSelectionClearsOnlyItsKind()
{
    PlaybackSnapshot snapshot = PlaybackSnapshot::opening(
        MediaGeneration{9},
        QStringLiteral("sample.mkv"));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(MediaLoadedEvent{}));

    TrackDescriptor videoTrack;
    videoTrack.id = 1;
    videoTrack.kind = TrackKind::Video;
    videoTrack.selected = true;
    TrackDescriptor audioTrack;
    audioTrack.id = 2;
    audioTrack.kind = TrackKind::Audio;
    audioTrack.selected = true;
    TrackDescriptor subtitleTrack;
    subtitleTrack.id = 3;
    subtitleTrack.kind = TrackKind::Subtitle;
    subtitleTrack.selected = true;

    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(TrackListChangedEvent{{videoTrack, audioTrack, subtitleTrack}}));
    QCOMPARE(*snapshot.tracks().selectedVideoId, qint64{1});
    QCOMPARE(*snapshot.tracks().selectedAudioId, qint64{2});
    QCOMPARE(*snapshot.tracks().selectedSubtitleId, qint64{3});

    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(SelectedAudioTrackChangedEvent{qint64{99}}));
    QCOMPARE(*snapshot.tracks().selectedVideoId, qint64{1});
    QVERIFY(!snapshot.tracks().selectedAudioId.has_value());
    QCOMPARE(*snapshot.tracks().selectedSubtitleId, qint64{3});
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());

    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(SelectedSubtitleTrackChangedEvent{std::nullopt}));
    QCOMPARE(*snapshot.tracks().selectedVideoId, qint64{1});
    QVERIFY(!snapshot.tracks().selectedAudioId.has_value());
    QVERIFY(!snapshot.tracks().selectedSubtitleId.has_value());
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());

    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(SelectedVideoTrackChangedEvent{qint64{98}}));
    QVERIFY(!snapshot.tracks().selectedVideoId.has_value());
    QVERIFY(!snapshot.tracks().selectedAudioId.has_value());
    QVERIFY(!snapshot.tracks().selectedSubtitleId.has_value());
    QCOMPARE(snapshot.tracks().tracks.size(), qsizetype{3});
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());
}

void PlaybackReducerTest::unavailablePauseAndBufferingDoNotInventState()
{
    PlaybackSnapshot snapshot = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(PauseChangedEvent{std::nullopt}));
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(BufferingChangedEvent{std::nullopt}));

    QVERIFY(snapshot.transport() == PlaybackTransportState::Playing);
    QVERIFY(snapshot.buffering().active);
    QCOMPARE(*snapshot.buffering().progressPercent, 63.0);
}

void PlaybackReducerTest::eofMarksEndedWithoutDiscardingMediaIdentity()
{
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(MediaEndedEvent{MediaEndReason::Eof}));

    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Ended);
    QVERIFY(next.transport() == PlaybackTransportState::Stopped);
    QCOMPARE(*next.media().source, QStringLiteral("next.mp4"));
    QCOMPARE(*next.media().title, QStringLiteral("Old title"));
    QCOMPARE(*next.timeline().durationSeconds, 120.0);
    QVERIFY(next.timeline().seeking.has_value());
    QVERIFY(!*next.timeline().seeking);
    QVERIFY(!next.buffering().active);
    QVERIFY(!next.buffering().cache.has_value());
    QCOMPARE(next.tracks().tracks.size(), qsizetype{1});
    QCOMPARE(next.chapters().chapters.size(), qsizetype{1});
    QVERIFY(next.streams().audio.has_value());
    QVERIFY(!next.failure().has_value());
}

void PlaybackReducerTest::eofPropertyMarksEndedAndSeekBackRecoversPaused()
{
    PlaybackSnapshot snapshot = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(EofReachedChangedEvent{true}));

    QVERIFY(snapshot.lifecycle() == PlaybackLifecycleState::Ended);
    QVERIFY(snapshot.transport() == PlaybackTransportState::Stopped);
    QCOMPARE(*snapshot.media().source, QStringLiteral("next.mp4"));
    QCOMPARE(*snapshot.media().title, QStringLiteral("Old title"));
    QCOMPARE(*snapshot.timeline().positionSeconds, 120.0);
    QCOMPARE(*snapshot.timeline().durationSeconds, 120.0);
    QVERIFY(*snapshot.timeline().seekable);
    QVERIFY(!snapshot.failure().has_value());

    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(PositionChangedEvent{95.0}));
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(EofReachedChangedEvent{false}));

    QVERIFY(snapshot.lifecycle() == PlaybackLifecycleState::Ready);
    QVERIFY(snapshot.transport() == PlaybackTransportState::Paused);
    QCOMPARE(*snapshot.media().source, QStringLiteral("next.mp4"));
    QCOMPARE(*snapshot.media().title, QStringLiteral("Old title"));
    QCOMPARE(*snapshot.timeline().positionSeconds, 95.0);
    QCOMPARE(*snapshot.timeline().durationSeconds, 120.0);
    QVERIFY(*snapshot.timeline().seekable);
    QVERIFY(!snapshot.failure().has_value());
}

void PlaybackReducerTest::stopClearsMediaScopedStateButPreservesControls()
{
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(MediaEndedEvent{MediaEndReason::Stopped}));

    QCOMPARE(next.generation().value(), quint64{17});
    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Empty);
    QVERIFY(next.transport() == PlaybackTransportState::Stopped);
    QVERIFY(!next.media().source.has_value());
    QVERIFY(!next.media().title.has_value());
    QVERIFY(!next.media().path.has_value());
    QVERIFY(!next.timeline().durationSeconds.has_value());
    QVERIFY(!next.buffering().active);
    QVERIFY(!next.buffering().cache.has_value());
    QVERIFY(next.tracks().tracks.isEmpty());
    QVERIFY(next.chapters().chapters.isEmpty());
    QVERIFY(!next.streams().audio.has_value());
    QCOMPARE(*next.controls().volumePercent, 72.0);
    QVERIFY(!*next.controls().muted);
    QCOMPARE(*next.controls().speed, 1.25);
    QVERIFY(!next.failure().has_value());
}

void PlaybackReducerTest::redirectReopensAndDropsOldMediaDetails()
{
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(MediaEndedEvent{MediaEndReason::Redirected}));

    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Opening);
    QVERIFY(next.transport() == PlaybackTransportState::Idle);
    QCOMPARE(*next.media().source, QStringLiteral("next.mp4"));
    QVERIFY(!next.media().title.has_value());
    QVERIFY(!next.media().path.has_value());
    QVERIFY(!next.timeline().positionSeconds.has_value());
    QVERIFY(next.tracks().tracks.isEmpty());
    QVERIFY(next.chapters().chapters.isEmpty());
    QVERIFY(!next.failure().has_value());
}

void PlaybackReducerTest::mediaFailureClearsStaleMediaStateAndKeepsSource()
{
    const PlaybackFailure failure{
        PlaybackFailureCategory::Media,
        -13,
        QStringLiteral("loading failed")};
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(MediaFailedEvent{failure}));

    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Failed);
    QVERIFY(next.transport() == PlaybackTransportState::Stopped);
    QCOMPARE(*next.media().source, QStringLiteral("next.mp4"));
    QVERIFY(!next.media().title.has_value());
    QVERIFY(!next.media().path.has_value());
    QVERIFY(!next.timeline().positionSeconds.has_value());
    QVERIFY(!next.timeline().durationSeconds.has_value());
    QVERIFY(!next.buffering().active);
    QVERIFY(!next.buffering().cache.has_value());
    QVERIFY(next.tracks().tracks.isEmpty());
    QVERIFY(next.chapters().chapters.isEmpty());
    QVERIFY(!next.streams().audio.has_value());
    QVERIFY(next.failure().has_value());
    QVERIFY(next.failure()->category == PlaybackFailureCategory::Media);
    QCOMPARE(next.failure()->backendCode, -13);
    QCOMPARE(next.failure()->diagnostic, QStringLiteral("loading failed"));
    QCOMPARE(*next.controls().volumePercent, 72.0);
}

void PlaybackReducerTest::backendShutdownMarksClosing()
{
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(PlaybackBackendShutdownEvent{}));

    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Closing);
    QVERIFY(next.transport() == PlaybackTransportState::Stopped);
    QCOMPARE(*next.media().source, QStringLiteral("next.mp4"));
    QVERIFY(next.timeline().seeking.has_value());
    QVERIFY(!*next.timeline().seeking);
    QVERIFY(!next.buffering().active);
    QVERIFY(!next.buffering().cache.has_value());
}

void PlaybackReducerTest::requestAndObservationEventsDoNotOwnSnapshotState()
{
    const PlaybackSnapshot current = populatedSnapshot();
    PlaybackSnapshot next = reducePlaybackSnapshot(
        current,
        makePlaybackEvent(CoreIdleChangedEvent{true}));
    next = reducePlaybackSnapshot(
        next,
        makePlaybackEvent(CommandReplyEvent{
            player::ids::RequestId{99},
            false,
            PlaybackFailure{
                PlaybackFailureCategory::Command,
                -5,
                QStringLiteral("command failed")}}));

    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Ready);
    QVERIFY(next.transport() == PlaybackTransportState::Playing);
    QCOMPARE(*next.media().title, QStringLiteral("Old title"));
    QCOMPARE(*next.timeline().positionSeconds, 41.0);
    QVERIFY(next.failure().has_value());
    QVERIFY(current.failure().has_value());
    QVERIFY(next.failure()->category == current.failure()->category);
    QCOMPARE(next.failure()->backendCode, current.failure()->backendCode);
    QCOMPARE(next.failure()->diagnostic, current.failure()->diagnostic);
}

void PlaybackReducerTest::protocolFailureIsRecordedWithoutInventingMediaFailure()
{
    const PlaybackFailure failure{
        PlaybackFailureCategory::Protocol,
        0,
        QStringLiteral("unexpected payload")};
    const PlaybackSnapshot next = reducePlaybackSnapshot(
        populatedSnapshot(),
        makePlaybackEvent(PlaybackFailureEvent{failure}));

    QVERIFY(next.lifecycle() == PlaybackLifecycleState::Ready);
    QVERIFY(next.transport() == PlaybackTransportState::Playing);
    QVERIFY(next.failure().has_value());
    QVERIFY(next.failure()->category == PlaybackFailureCategory::Protocol);
    QCOMPARE(next.failure()->diagnostic, QStringLiteral("unexpected payload"));
}

} // namespace player::playback::domain

QTEST_GUILESS_MAIN(player::playback::domain::PlaybackReducerTest)
#include "playback_reducer_test.moc"
