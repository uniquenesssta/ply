#include "presentation/viewmodels/player/transport/player_transport_view_model.h"

#include <QSignalSpy>
#include <QtTest>

namespace player::presentation {
namespace {

using player::playback::domain::PlaybackLifecycleState;
using player::playback::domain::PlaybackSnapshot;
using player::playback::domain::PlaybackSnapshotState;
using player::playback::domain::PlaybackTransportState;

PlaybackSnapshot snapshotFor(
    PlaybackLifecycleState lifecycle,
    PlaybackTransportState transport)
{
    PlaybackSnapshotState state;
    state.lifecycle = lifecycle;
    state.transport = transport;
    return PlaybackSnapshot{state};
}

} // namespace

class PlayerTransportViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void snapshotProjectionMatchesSelectors();
    void intentsRespectProjectedEnableState();
    void playlistNavigationRemainsDisabledUntilPlaylistIntegration();
};

void PlayerTransportViewModelTest::snapshotProjectionMatchesSelectors()
{
    PlayerTransportViewModel viewModel;
    QSignalSpy stateSpy(&viewModel, &PlayerTransportViewModel::stateChanged);

    QVERIFY(!viewModel.canPlay());
    QVERIFY(!viewModel.canPause());
    QVERIFY(!viewModel.canStop());
    QVERIFY(!viewModel.isPlaying());

    const PlaybackSnapshot readyIdle = snapshotFor(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Idle);
    viewModel.acceptSnapshot(readyIdle);
    QCOMPARE(stateSpy.count(), 1);
    QVERIFY(viewModel.canPlay());
    QVERIFY(!viewModel.canPause());
    QVERIFY(viewModel.canStop());
    QVERIFY(!viewModel.isPlaying());

    viewModel.acceptSnapshot(readyIdle);
    QCOMPARE(stateSpy.count(), 1);

    viewModel.acceptSnapshot(snapshotFor(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing));
    QCOMPARE(stateSpy.count(), 2);
    QVERIFY(!viewModel.canPlay());
    QVERIFY(viewModel.canPause());
    QVERIFY(viewModel.canStop());
    QVERIFY(viewModel.isPlaying());

    viewModel.acceptSnapshot(snapshotFor(
        PlaybackLifecycleState::Ended,
        PlaybackTransportState::Stopped));
    QCOMPARE(stateSpy.count(), 3);
    QVERIFY(!viewModel.canPlay());
    QVERIFY(!viewModel.canPause());
    QVERIFY(!viewModel.canStop());
    QVERIFY(!viewModel.isPlaying());
}

void PlayerTransportViewModelTest::intentsRespectProjectedEnableState()
{
    PlayerTransportViewModel viewModel;
    QSignalSpy playSpy(&viewModel, &PlayerTransportViewModel::playRequested);
    QSignalSpy pauseSpy(&viewModel, &PlayerTransportViewModel::pauseRequested);
    QSignalSpy stopSpy(&viewModel, &PlayerTransportViewModel::stopRequested);

    QVERIFY(!viewModel.requestPlay());
    QVERIFY(!viewModel.requestPause());
    QVERIFY(!viewModel.requestTogglePlayPause());
    QVERIFY(!viewModel.requestStop());
    QCOMPARE(playSpy.count(), 0);
    QCOMPARE(pauseSpy.count(), 0);
    QCOMPARE(stopSpy.count(), 0);

    viewModel.acceptSnapshot(snapshotFor(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Paused));
    QVERIFY(viewModel.requestPlay());
    QVERIFY(viewModel.requestTogglePlayPause());
    QVERIFY(viewModel.requestStop());
    QCOMPARE(playSpy.count(), 2);
    QCOMPARE(pauseSpy.count(), 0);
    QCOMPARE(stopSpy.count(), 1);

    viewModel.acceptSnapshot(snapshotFor(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing));
    QVERIFY(!viewModel.requestPlay());
    QVERIFY(viewModel.requestPause());
    QVERIFY(viewModel.requestTogglePlayPause());
    QCOMPARE(playSpy.count(), 2);
    QCOMPARE(pauseSpy.count(), 2);
}

void PlayerTransportViewModelTest::playlistNavigationRemainsDisabledUntilPlaylistIntegration()
{
    PlayerTransportViewModel viewModel;
    QVERIFY(!viewModel.canPrevious());
    QVERIFY(!viewModel.canNext());
    QVERIFY(!viewModel.requestPrevious());
    QVERIFY(!viewModel.requestNext());
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::PlayerTransportViewModelTest)
#include "player_transport_view_model_test.moc"
