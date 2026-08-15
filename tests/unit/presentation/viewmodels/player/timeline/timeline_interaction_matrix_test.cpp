#include "presentation/viewmodels/player/timeline/player_timeline_view_model.h"

#include <QSignalSpy>
#include <QtTest>

#include <cmath>

namespace player::presentation {
namespace {

using player::playback::domain::MediaGeneration;
using player::playback::domain::PlaybackLifecycleState;
using player::playback::domain::PlaybackSnapshot;
using player::playback::domain::PlaybackSnapshotState;
using player::playback::domain::PlaybackTransportState;

bool fuzzyEqual(double left, double right)
{
    return std::abs(left - right) <= 0.000001;
}

PlaybackSnapshot timelineState(
    quint64 generation,
    double position,
    double duration,
    bool seekable,
    PlaybackLifecycleState lifecycle = PlaybackLifecycleState::Ready,
    PlaybackTransportState transport = PlaybackTransportState::Playing,
    bool buffering = false)
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{generation};
    state.lifecycle = lifecycle;
    state.transport = transport;
    state.timeline.positionSeconds = position;
    state.timeline.durationSeconds = duration;
    state.timeline.seekable = seekable;
    state.timeline.seeking = false;
    state.buffering.active = buffering;
    return PlaybackSnapshot{state};
}

} // namespace

class TimelineInteractionMatrixTest final : public QObject
{
    Q_OBJECT

private slots:
    void playingPausedAndBufferingScrubKeepPreviewOwnership();
    void rapidRelativeSeekCoalescesToOneCommand();
    void sustainedRelativeInputFlushesBoundedBatches();
    void relativeSeekClampsAtTimelineBounds();
    void generationChangeCancelsQueuedRelativeSeek();
    void endedMediaCanSeekEarlierWithoutReplayIntent();
    void rejectedRelativeSeekRestoresActualPosition();
};

void TimelineInteractionMatrixTest::playingPausedAndBufferingScrubKeepPreviewOwnership()
{
    PlayerTimelineViewModel viewModel;

    viewModel.acceptSnapshot(timelineState(
        1,
        10.0,
        100.0,
        true,
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing));
    QVERIFY(viewModel.canSeek());
    QVERIFY(viewModel.beginScrub(0.1));
    QVERIFY(viewModel.updateScrub(0.7));
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.7));

    viewModel.acceptSnapshot(timelineState(
        1,
        12.0,
        100.0,
        true,
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Paused));
    QVERIFY(viewModel.canSeek());
    QVERIFY(viewModel.isScrubbing());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.7));

    viewModel.acceptSnapshot(timelineState(
        1,
        13.0,
        100.0,
        true,
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Paused,
        true));
    QVERIFY(viewModel.canSeek());
    QVERIFY(viewModel.isScrubbing());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.7));
    QVERIFY(!viewModel.requestRelativeSeek(viewModel.relativeSeekStepSeconds()));

    QVERIFY(viewModel.cancelScrub());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.13));
}

void TimelineInteractionMatrixTest::rapidRelativeSeekCoalescesToOneCommand()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy relativeSpy(&viewModel, &PlayerTimelineViewModel::relativeSeekRequested);

    viewModel.acceptSnapshot(timelineState(2, 40.0, 100.0, true));
    QCOMPARE(viewModel.relativeSeekStepSeconds(), 5.0);

    for (int index = 0; index < 10; ++index) {
        QVERIFY(viewModel.requestRelativeSeek(viewModel.relativeSeekStepSeconds()));
    }

    QVERIFY(viewModel.seekPending());
    QTRY_COMPARE_WITH_TIMEOUT(relativeSpy.count(), 1, 1000);
    QVERIFY(fuzzyEqual(relativeSpy.at(0).at(0).toDouble(), 50.0));
    QVERIFY(viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.9));
    QCOMPARE(viewModel.positionText(), QStringLiteral("00:01:30"));

    viewModel.acceptSnapshot(timelineState(2, 90.2, 100.0, true));
    QVERIFY(!viewModel.seekPending());
}

void TimelineInteractionMatrixTest::sustainedRelativeInputFlushesBoundedBatches()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy relativeSpy(&viewModel, &PlayerTimelineViewModel::relativeSeekRequested);

    viewModel.acceptSnapshot(timelineState(8, 100.0, 500.0, true));
    for (int index = 0; index < 12; ++index) {
        QVERIFY(viewModel.requestRelativeSeek(viewModel.relativeSeekStepSeconds()));
        QTest::qWait(20);
    }

    QTest::qWait(180);
    QVERIFY(relativeSpy.count() >= 2);
    QVERIFY(relativeSpy.count() <= 4);

    double submittedDelta = 0.0;
    for (int index = 0; index < relativeSpy.count(); ++index) {
        submittedDelta += relativeSpy.at(index).at(0).toDouble();
    }
    QVERIFY(fuzzyEqual(submittedDelta, 60.0));
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 160.0 / 500.0));
}

void TimelineInteractionMatrixTest::relativeSeekClampsAtTimelineBounds()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy relativeSpy(&viewModel, &PlayerTimelineViewModel::relativeSeekRequested);

    viewModel.acceptSnapshot(timelineState(3, 2.0, 100.0, true));
    QVERIFY(viewModel.requestRelativeSeek(-viewModel.relativeSeekStepSeconds()));
    QTRY_COMPARE_WITH_TIMEOUT(relativeSpy.count(), 1, 1000);
    QVERIFY(fuzzyEqual(relativeSpy.at(0).at(0).toDouble(), -2.0));
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.0));

    viewModel.acceptSnapshot(timelineState(3, 0.0, 100.0, true));
    QVERIFY(!viewModel.seekPending());

    viewModel.acceptSnapshot(timelineState(3, 98.0, 100.0, true));
    QVERIFY(viewModel.requestRelativeSeek(viewModel.relativeSeekStepSeconds()));
    QTRY_COMPARE_WITH_TIMEOUT(relativeSpy.count(), 2, 1000);
    QVERIFY(fuzzyEqual(relativeSpy.at(1).at(0).toDouble(), 2.0));
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 1.0));
}

void TimelineInteractionMatrixTest::generationChangeCancelsQueuedRelativeSeek()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy relativeSpy(&viewModel, &PlayerTimelineViewModel::relativeSeekRequested);

    viewModel.acceptSnapshot(timelineState(4, 20.0, 100.0, true));
    QVERIFY(viewModel.requestRelativeSeek(viewModel.relativeSeekStepSeconds()));
    QVERIFY(viewModel.seekPending());

    viewModel.acceptSnapshot(timelineState(5, 7.0, 200.0, true));
    QVERIFY(!viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 7.0 / 200.0));

    QTest::qWait(150);
    QCOMPARE(relativeSpy.count(), 0);
}

void TimelineInteractionMatrixTest::endedMediaCanSeekEarlierWithoutReplayIntent()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy relativeSpy(&viewModel, &PlayerTimelineViewModel::relativeSeekRequested);

    viewModel.acceptSnapshot(timelineState(
        6,
        100.0,
        100.0,
        true,
        PlaybackLifecycleState::Ended,
        PlaybackTransportState::Stopped));

    QVERIFY(viewModel.canSeek());
    QVERIFY(viewModel.requestRelativeSeek(-viewModel.relativeSeekStepSeconds()));
    QTRY_COMPARE_WITH_TIMEOUT(relativeSpy.count(), 1, 1000);
    QVERIFY(fuzzyEqual(relativeSpy.at(0).at(0).toDouble(), -5.0));
    QVERIFY(viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.95));
    QCOMPARE(viewModel.positionText(), QStringLiteral("00:01:35"));
}

void TimelineInteractionMatrixTest::rejectedRelativeSeekRestoresActualPosition()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy relativeSpy(&viewModel, &PlayerTimelineViewModel::relativeSeekRequested);

    viewModel.acceptSnapshot(timelineState(7, 30.0, 100.0, true));
    QVERIFY(viewModel.requestRelativeSeek(viewModel.relativeSeekStepSeconds()));
    QTRY_COMPARE_WITH_TIMEOUT(relativeSpy.count(), 1, 1000);
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.35));
    QVERIFY(viewModel.seekPending());

    QVERIFY(viewModel.rejectPendingSeek());
    QVERIFY(!viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.3));
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::TimelineInteractionMatrixTest)
#include "timeline_interaction_matrix_test.moc"
