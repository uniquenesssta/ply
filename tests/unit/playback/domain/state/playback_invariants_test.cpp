#include "playback/domain/state/playback_invariants.h"
#include "playback/domain/state/playback_reducer.h"

#include <QtTest/QTest>

#include <algorithm>
#include <utility>

namespace player::playback::domain {
namespace {

bool containsViolation(
    const PlaybackInvariantViolations& violations,
    PlaybackInvariantViolation expected)
{
    return std::find(violations.cbegin(), violations.cend(), expected) != violations.cend();
}

PlaybackEvent makePlaybackEvent(PlaybackEventPayload payload)
{
    return PlaybackEvent{std::move(payload)};
}

PlaybackSnapshot readySnapshot(MediaGeneration generation = MediaGeneration{1})
{
    PlaybackSnapshotState state;
    state.generation = generation;
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.transport = PlaybackTransportState::Paused;
    state.media.source = QStringLiteral("sample.mp4");
    state.timeline.positionSeconds = 10.0;
    state.timeline.durationSeconds = 90.0;
    state.timeline.seekable = true;
    state.timeline.seeking = false;
    state.controls.volumePercent = 80.0;
    state.controls.muted = false;
    state.controls.speed = 1.0;
    return PlaybackSnapshot{std::move(state)};
}

} // namespace

class PlaybackInvariantsTest final : public QObject
{
    Q_OBJECT

private slots:
    void acceptsCanonicalSnapshotStates();
    void reducerMainPathProducesValidSnapshots();
    void detectsMediaStateWithoutMedia();
    void detectsMissingActiveMediaIdentity();
    void detectsLifecycleTransportMismatch();
    void detectsFailedStateWithoutFailure();
    void detectsBufferingOutsideActiveMedia();
    void detectsSeekingConflicts();
    void detectsSelectedTrackOutsideCurrentList();
    void detectsGenerationRegression();
};

void PlaybackInvariantsTest::acceptsCanonicalSnapshotStates()
{
    QVERIFY(checkPlaybackSnapshotInvariants(PlaybackSnapshot{}).empty());
    QVERIFY(checkPlaybackSnapshotInvariants(PlaybackSnapshot::stopped(MediaGeneration{2})).empty());
    QVERIFY(checkPlaybackSnapshotInvariants(
        PlaybackSnapshot::opening(MediaGeneration{3}, QStringLiteral("sample.mp4"))).empty());
    QVERIFY(checkPlaybackSnapshotInvariants(readySnapshot(MediaGeneration{4})).empty());

    PlaybackSnapshotState ended = readySnapshot(MediaGeneration{5}).state();
    ended.lifecycle = PlaybackLifecycleState::Ended;
    ended.transport = PlaybackTransportState::Stopped;
    ended.timeline.seeking = false;
    QVERIFY(checkPlaybackSnapshotInvariants(PlaybackSnapshot{std::move(ended)}).empty());

    PlaybackSnapshotState failed = readySnapshot(MediaGeneration{6}).state();
    failed.lifecycle = PlaybackLifecycleState::Failed;
    failed.transport = PlaybackTransportState::Stopped;
    failed.timeline = {};
    failed.buffering = {};
    failed.failure = PlaybackFailure{
        PlaybackFailureCategory::Media,
        -13,
        QStringLiteral("loading failed")};
    QVERIFY(checkPlaybackSnapshotInvariants(PlaybackSnapshot{std::move(failed)}).empty());
}

void PlaybackInvariantsTest::reducerMainPathProducesValidSnapshots()
{
    PlaybackSnapshot snapshot = PlaybackSnapshot::opening(
        MediaGeneration{7},
        QStringLiteral("sample.mp4"));
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());

    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(MediaLoadedEvent{}));
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());

    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(PauseChangedEvent{false}));
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());

    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(SeekableChangedEvent{true}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(SeekingChangedEvent{true}));
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());

    TrackDescriptor audio;
    audio.id = 2;
    audio.kind = TrackKind::Audio;
    audio.selected = true;
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(TrackListChangedEvent{{audio}}));
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());

    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(SeekingChangedEvent{false}));
    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(BufferingChangedEvent{true}));
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());

    snapshot = reducePlaybackSnapshot(snapshot, makePlaybackEvent(BufferingChangedEvent{false}));
    snapshot = reducePlaybackSnapshot(
        snapshot,
        makePlaybackEvent(MediaEndedEvent{MediaEndReason::Eof}));
    QVERIFY(checkPlaybackSnapshotInvariants(snapshot).empty());
}

void PlaybackInvariantsTest::detectsMediaStateWithoutMedia()
{
    PlaybackSnapshotState state;
    state.lifecycle = PlaybackLifecycleState::Empty;
    state.transport = PlaybackTransportState::Stopped;
    state.media.title = QStringLiteral("stale title");
    state.timeline.positionSeconds = 12.0;
    TrackDescriptor audio;
    audio.id = 3;
    audio.kind = TrackKind::Audio;
    state.tracks.tracks.append(audio);

    const PlaybackInvariantViolations violations = checkPlaybackSnapshotInvariants(
        PlaybackSnapshot{std::move(state)});

    QVERIFY(containsViolation(
        violations,
        PlaybackInvariantViolation::EmptyHasMediaScopedState));
}

void PlaybackInvariantsTest::detectsMissingActiveMediaIdentity()
{
    PlaybackSnapshotState state;
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.transport = PlaybackTransportState::Paused;

    const PlaybackInvariantViolations violations = checkPlaybackSnapshotInvariants(
        PlaybackSnapshot{std::move(state)});

    QVERIFY(containsViolation(
        violations,
        PlaybackInvariantViolation::ActiveMediaMissingGeneration));
    QVERIFY(containsViolation(
        violations,
        PlaybackInvariantViolation::ActiveMediaMissingSource));
}

void PlaybackInvariantsTest::detectsLifecycleTransportMismatch()
{
    PlaybackSnapshotState state = readySnapshot().state();
    state.lifecycle = PlaybackLifecycleState::Opening;
    state.transport = PlaybackTransportState::Playing;

    const PlaybackInvariantViolations violations = checkPlaybackSnapshotInvariants(
        PlaybackSnapshot{std::move(state)});

    QVERIFY(containsViolation(
        violations,
        PlaybackInvariantViolation::LifecycleTransportMismatch));
}

void PlaybackInvariantsTest::detectsFailedStateWithoutFailure()
{
    PlaybackSnapshotState state = readySnapshot().state();
    state.lifecycle = PlaybackLifecycleState::Failed;
    state.transport = PlaybackTransportState::Stopped;
    state.timeline = {};
    state.failure.reset();

    const PlaybackInvariantViolations violations = checkPlaybackSnapshotInvariants(
        PlaybackSnapshot{std::move(state)});

    QVERIFY(containsViolation(
        violations,
        PlaybackInvariantViolation::FailedWithoutFailure));
}

void PlaybackInvariantsTest::detectsBufferingOutsideActiveMedia()
{
    PlaybackSnapshotState state = readySnapshot().state();
    state.lifecycle = PlaybackLifecycleState::Ended;
    state.transport = PlaybackTransportState::Stopped;
    state.timeline.seeking = false;
    state.buffering.active = true;

    const PlaybackInvariantViolations violations = checkPlaybackSnapshotInvariants(
        PlaybackSnapshot{std::move(state)});

    QVERIFY(containsViolation(
        violations,
        PlaybackInvariantViolation::BufferingOutsideActiveMedia));
}

void PlaybackInvariantsTest::detectsSeekingConflicts()
{
    PlaybackSnapshotState outsideReady = readySnapshot().state();
    outsideReady.lifecycle = PlaybackLifecycleState::Opening;
    outsideReady.transport = PlaybackTransportState::Idle;
    outsideReady.timeline.seeking = true;

    const PlaybackInvariantViolations outsideReadyViolations = checkPlaybackSnapshotInvariants(
        PlaybackSnapshot{std::move(outsideReady)});
    QVERIFY(containsViolation(
        outsideReadyViolations,
        PlaybackInvariantViolation::SeekingOutsideReadyMedia));

    PlaybackSnapshotState notSeekable = readySnapshot().state();
    notSeekable.timeline.seekable = false;
    notSeekable.timeline.seeking = true;

    const PlaybackInvariantViolations notSeekableViolations = checkPlaybackSnapshotInvariants(
        PlaybackSnapshot{std::move(notSeekable)});
    QVERIFY(containsViolation(
        notSeekableViolations,
        PlaybackInvariantViolation::SeekingWhenNotSeekable));
}

void PlaybackInvariantsTest::detectsSelectedTrackOutsideCurrentList()
{
    PlaybackSnapshotState state = readySnapshot().state();
    TrackDescriptor audio;
    audio.id = 2;
    audio.kind = TrackKind::Audio;
    state.tracks.tracks.append(audio);
    state.tracks.selectedAudioId = 99;

    const PlaybackInvariantViolations violations = checkPlaybackSnapshotInvariants(
        PlaybackSnapshot{std::move(state)});

    QVERIFY(containsViolation(
        violations,
        PlaybackInvariantViolation::SelectedTrackMissingFromCurrentMedia));
}

void PlaybackInvariantsTest::detectsGenerationRegression()
{
    const PlaybackSnapshot previous = readySnapshot(MediaGeneration{9});

    QVERIFY(checkPlaybackTransitionInvariants(
        previous,
        readySnapshot(MediaGeneration{9})).empty());
    QVERIFY(checkPlaybackTransitionInvariants(
        previous,
        readySnapshot(MediaGeneration{10})).empty());

    const PlaybackInvariantViolations lowerGeneration = checkPlaybackTransitionInvariants(
        previous,
        readySnapshot(MediaGeneration{8}));
    QVERIFY(containsViolation(
        lowerGeneration,
        PlaybackInvariantViolation::GenerationRegressed));

    PlaybackSnapshotState invalidGeneration = readySnapshot(MediaGeneration{9}).state();
    invalidGeneration.generation = MediaGeneration{};
    const PlaybackInvariantViolations invalidGenerationViolations = checkPlaybackTransitionInvariants(
        previous,
        PlaybackSnapshot{std::move(invalidGeneration)});
    QVERIFY(containsViolation(
        invalidGenerationViolations,
        PlaybackInvariantViolation::GenerationRegressed));
}

} // namespace player::playback::domain

QTEST_GUILESS_MAIN(player::playback::domain::PlaybackInvariantsTest)
#include "playback_invariants_test.moc"
