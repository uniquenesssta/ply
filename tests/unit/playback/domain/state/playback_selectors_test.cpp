#include "playback/domain/state/playback_selectors.h"

#include <QtTest>

namespace player::playback::domain {
namespace {

PlaybackSnapshot snapshotFor(
    PlaybackLifecycleState lifecycle,
    PlaybackTransportState transport)
{
    PlaybackSnapshotState state;
    state.lifecycle = lifecycle;
    state.transport = transport;
    return PlaybackSnapshot{state};
}

PlaybackSnapshot seekSnapshotFor(
    PlaybackLifecycleState lifecycle,
    std::optional<bool> seekable)
{
    PlaybackSnapshotState state;
    state.lifecycle = lifecycle;
    state.timeline.seekable = seekable;
    return PlaybackSnapshot{state};
}

} // namespace

class PlaybackSelectorsTest final : public QObject
{
    Q_OBJECT

private slots:
    void lifecycleAndTransportMatrixIsConservative();
    void seekCapabilityRequiresReadyAndExplicitBackendSupport();
};

void PlaybackSelectorsTest::lifecycleAndTransportMatrixIsConservative()
{
    const PlaybackSnapshot empty = snapshotFor(
        PlaybackLifecycleState::Empty,
        PlaybackTransportState::Idle);
    QVERIFY(!selectors::canPlay(empty));
    QVERIFY(!selectors::canPause(empty));
    QVERIFY(!selectors::canStop(empty));
    QVERIFY(!selectors::isPlaying(empty));

    const PlaybackSnapshot opening = snapshotFor(
        PlaybackLifecycleState::Opening,
        PlaybackTransportState::Idle);
    QVERIFY(!selectors::canPlay(opening));
    QVERIFY(!selectors::canPause(opening));
    QVERIFY(selectors::canStop(opening));
    QVERIFY(!selectors::isPlaying(opening));

    const PlaybackSnapshot readyIdle = snapshotFor(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Idle);
    QVERIFY(selectors::canPlay(readyIdle));
    QVERIFY(!selectors::canPause(readyIdle));
    QVERIFY(selectors::canStop(readyIdle));
    QVERIFY(!selectors::isPlaying(readyIdle));

    const PlaybackSnapshot playing = snapshotFor(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing);
    QVERIFY(!selectors::canPlay(playing));
    QVERIFY(selectors::canPause(playing));
    QVERIFY(selectors::canStop(playing));
    QVERIFY(selectors::isPlaying(playing));

    const PlaybackSnapshot paused = snapshotFor(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Paused);
    QVERIFY(selectors::canPlay(paused));
    QVERIFY(!selectors::canPause(paused));
    QVERIFY(selectors::canStop(paused));
    QVERIFY(!selectors::isPlaying(paused));

    const PlaybackSnapshot readyStopped = snapshotFor(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Stopped);
    QVERIFY(!selectors::canPlay(readyStopped));
    QVERIFY(!selectors::canPause(readyStopped));
    QVERIFY(!selectors::canStop(readyStopped));
    QVERIFY(!selectors::isPlaying(readyStopped));

    const PlaybackSnapshot ended = snapshotFor(
        PlaybackLifecycleState::Ended,
        PlaybackTransportState::Stopped);
    QVERIFY(!selectors::canPlay(ended));
    QVERIFY(!selectors::canPause(ended));
    QVERIFY(!selectors::canStop(ended));
    QVERIFY(!selectors::isPlaying(ended));

    const PlaybackSnapshot failed = snapshotFor(
        PlaybackLifecycleState::Failed,
        PlaybackTransportState::Stopped);
    QVERIFY(!selectors::canPlay(failed));
    QVERIFY(!selectors::canPause(failed));
    QVERIFY(!selectors::canStop(failed));
    QVERIFY(!selectors::isPlaying(failed));

    const PlaybackSnapshot closing = snapshotFor(
        PlaybackLifecycleState::Closing,
        PlaybackTransportState::Stopped);
    QVERIFY(!selectors::canPlay(closing));
    QVERIFY(!selectors::canPause(closing));
    QVERIFY(!selectors::canStop(closing));
    QVERIFY(!selectors::isPlaying(closing));
}

void PlaybackSelectorsTest::seekCapabilityRequiresReadyAndExplicitBackendSupport()
{
    QVERIFY(!selectors::canSeek(seekSnapshotFor(
        PlaybackLifecycleState::Empty,
        true)));
    QVERIFY(!selectors::canSeek(seekSnapshotFor(
        PlaybackLifecycleState::Opening,
        true)));
    QVERIFY(!selectors::canSeek(seekSnapshotFor(
        PlaybackLifecycleState::Ready,
        std::nullopt)));
    QVERIFY(!selectors::canSeek(seekSnapshotFor(
        PlaybackLifecycleState::Ready,
        false)));
    QVERIFY(selectors::canSeek(seekSnapshotFor(
        PlaybackLifecycleState::Ready,
        true)));
    QVERIFY(!selectors::canSeek(seekSnapshotFor(
        PlaybackLifecycleState::Ended,
        true)));
}

} // namespace player::playback::domain

QTEST_GUILESS_MAIN(player::playback::domain::PlaybackSelectorsTest)
#include "playback_selectors_test.moc"
