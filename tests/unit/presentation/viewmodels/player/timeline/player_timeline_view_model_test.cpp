#include "presentation/viewmodels/player/timeline/player_timeline_view_model.h"

#include <QSignalSpy>
#include <QtTest>

#include <cmath>
#include <optional>

namespace player::presentation {
namespace {

using player::playback::domain::MediaGeneration;
using player::playback::domain::PlaybackLifecycleState;
using player::playback::domain::PlaybackSnapshot;
using player::playback::domain::PlaybackSnapshotState;

bool fuzzyEqual(double left, double right)
{
    return std::abs(left - right) <= 0.000001;
}

PlaybackSnapshot timelineSnapshot(
    quint64 generation,
    std::optional<double> position,
    std::optional<double> duration,
    std::optional<bool> seekable,
    std::optional<bool> seeking = false,
    PlaybackLifecycleState lifecycle = PlaybackLifecycleState::Ready)
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{generation};
    state.lifecycle = lifecycle;
    state.timeline.positionSeconds = position;
    state.timeline.durationSeconds = duration;
    state.timeline.seekable = seekable;
    state.timeline.seeking = seeking;
    return PlaybackSnapshot{state};
}

} // namespace

class PlayerTimelineViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void snapshotProjectsPositionDurationAndTimecodes();
    void scrubPreviewIgnoresBackgroundPositionUntilAcknowledged();
    void pendingCommitKeepsAbsoluteTargetAcrossDurationRefresh();
    void cancelRestoresActualWithoutSeek();
    void generationChangeCancelsScrub();
    void nonSeekableAndUnknownDurationBlockInteraction();
    void backendSeekingCycleAcknowledgesPendingCommit();
    void rejectedSubmissionReleasesPendingPreview();
};

void PlayerTimelineViewModelTest::snapshotProjectsPositionDurationAndTimecodes()
{
    PlayerTimelineViewModel viewModel;

    QCOMPARE(viewModel.positionText(), QStringLiteral("--:--:--"));
    QCOMPARE(viewModel.durationText(), QStringLiteral("--:--:--"));
    QVERIFY(!viewModel.canSeek());

    viewModel.acceptSnapshot(timelineSnapshot(
        1,
        60.0,
        3661.0,
        true));

    QVERIFY(viewModel.canSeek());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 60.0 / 3661.0));
    QVERIFY(fuzzyEqual(viewModel.durationSeconds(), 3661.0));
    QCOMPARE(viewModel.positionText(), QStringLiteral("00:01:00"));
    QCOMPARE(viewModel.durationText(), QStringLiteral("01:01:01"));
}

void PlayerTimelineViewModelTest::scrubPreviewIgnoresBackgroundPositionUntilAcknowledged()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy seekSpy(&viewModel, &PlayerTimelineViewModel::seekRequested);

    viewModel.acceptSnapshot(timelineSnapshot(2, 10.0, 100.0, true));
    QVERIFY(viewModel.beginScrub(0.2));
    QVERIFY(viewModel.updateScrub(0.8));
    QVERIFY(viewModel.isScrubbing());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.8));
    QCOMPARE(viewModel.positionText(), QStringLiteral("00:01:20"));

    viewModel.acceptSnapshot(timelineSnapshot(2, 11.0, 100.0, true));
    QVERIFY(viewModel.isScrubbing());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.8));

    QVERIFY(viewModel.commitScrub(0.75));
    QVERIFY(!viewModel.isScrubbing());
    QVERIFY(viewModel.seekPending());
    QCOMPARE(seekSpy.count(), 1);
    QVERIFY(fuzzyEqual(seekSpy.at(0).at(0).toDouble(), 75.0));

    viewModel.acceptSnapshot(timelineSnapshot(2, 12.0, 100.0, true));
    QVERIFY(viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.75));

    viewModel.acceptSnapshot(timelineSnapshot(2, 75.2, 100.0, true));
    QVERIFY(!viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.752));
    QCOMPARE(seekSpy.count(), 1);
}

void PlayerTimelineViewModelTest::pendingCommitKeepsAbsoluteTargetAcrossDurationRefresh()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy seekSpy(&viewModel, &PlayerTimelineViewModel::seekRequested);

    viewModel.acceptSnapshot(timelineSnapshot(9, 10.0, 100.0, true));
    QVERIFY(viewModel.beginScrub(0.1));
    QVERIFY(viewModel.commitScrub(0.75));
    QCOMPARE(seekSpy.count(), 1);
    QVERIFY(fuzzyEqual(seekSpy.at(0).at(0).toDouble(), 75.0));

    viewModel.acceptSnapshot(timelineSnapshot(9, 12.0, 200.0, true));
    QVERIFY(viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.375));
    QCOMPARE(viewModel.positionText(), QStringLiteral("00:01:15"));

    viewModel.acceptSnapshot(timelineSnapshot(9, 75.1, 200.0, true));
    QVERIFY(!viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 75.1 / 200.0));
    QCOMPARE(seekSpy.count(), 1);
}

void PlayerTimelineViewModelTest::cancelRestoresActualWithoutSeek()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy seekSpy(&viewModel, &PlayerTimelineViewModel::seekRequested);

    viewModel.acceptSnapshot(timelineSnapshot(3, 25.0, 100.0, true));
    QVERIFY(viewModel.beginScrub(0.25));
    QVERIFY(viewModel.updateScrub(0.9));
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.9));
    QVERIFY(viewModel.cancelScrub());
    QVERIFY(!viewModel.isScrubbing());
    QVERIFY(!viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.25));
    QCOMPARE(seekSpy.count(), 0);
}

void PlayerTimelineViewModelTest::generationChangeCancelsScrub()
{
    PlayerTimelineViewModel viewModel;
    QSignalSpy seekSpy(&viewModel, &PlayerTimelineViewModel::seekRequested);

    viewModel.acceptSnapshot(timelineSnapshot(4, 20.0, 100.0, true));
    QVERIFY(viewModel.beginScrub(0.2));
    QVERIFY(viewModel.updateScrub(0.8));

    viewModel.acceptSnapshot(timelineSnapshot(5, 5.0, 200.0, true));
    QVERIFY(!viewModel.isScrubbing());
    QVERIFY(!viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.025));
    QCOMPARE(seekSpy.count(), 0);
}

void PlayerTimelineViewModelTest::nonSeekableAndUnknownDurationBlockInteraction()
{
    PlayerTimelineViewModel viewModel;

    viewModel.acceptSnapshot(timelineSnapshot(6, 10.0, 100.0, false));
    QVERIFY(!viewModel.canSeek());
    QVERIFY(!viewModel.beginScrub(0.5));

    viewModel.acceptSnapshot(timelineSnapshot(
        6,
        10.0,
        std::nullopt,
        true));
    QVERIFY(!viewModel.canSeek());
    QVERIFY(!viewModel.beginScrub(0.5));
    QCOMPARE(viewModel.durationText(), QStringLiteral("--:--:--"));
}

void PlayerTimelineViewModelTest::backendSeekingCycleAcknowledgesPendingCommit()
{
    PlayerTimelineViewModel viewModel;

    viewModel.acceptSnapshot(timelineSnapshot(7, 10.0, 100.0, true));
    QVERIFY(viewModel.beginScrub(0.1));
    QVERIFY(viewModel.commitScrub(0.8));
    QVERIFY(viewModel.seekPending());

    viewModel.acceptSnapshot(timelineSnapshot(7, 10.0, 100.0, true, true));
    QVERIFY(viewModel.seekPending());
    QVERIFY(viewModel.backendSeeking());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.8));

    viewModel.acceptSnapshot(timelineSnapshot(7, 11.0, 100.0, true, false));
    QVERIFY(!viewModel.seekPending());
    QVERIFY(!viewModel.backendSeeking());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.11));
}

void PlayerTimelineViewModelTest::rejectedSubmissionReleasesPendingPreview()
{
    PlayerTimelineViewModel viewModel;

    viewModel.acceptSnapshot(timelineSnapshot(8, 30.0, 100.0, true));
    QVERIFY(viewModel.beginScrub(0.3));
    QVERIFY(viewModel.commitScrub(0.6));
    QVERIFY(viewModel.seekPending());
    QVERIFY(viewModel.rejectPendingSeek());
    QVERIFY(!viewModel.seekPending());
    QVERIFY(fuzzyEqual(viewModel.displayedNormalized(), 0.3));
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::PlayerTimelineViewModelTest)
#include "player_timeline_view_model_test.moc"
