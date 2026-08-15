#include "presentation/viewmodels/player/status/player_status_view_model.h"

#include "playback/domain/errors/playback_failure.h"
#include "playback/domain/state/media_generation.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QSignalSpy>
#include <QtTest>

#include <optional>

namespace player::presentation {
namespace {

player::playback::domain::PlaybackSnapshot makeSnapshot(
    player::playback::domain::PlaybackLifecycleState lifecycle,
    player::playback::domain::PlaybackTransportState transport,
    bool bufferingActive = false,
    std::optional<double> bufferingPercent = std::nullopt)
{
    using namespace player::playback::domain;

    PlaybackSnapshotState state;
    state.lifecycle = lifecycle;
    state.transport = transport;
    state.buffering.active = bufferingActive;
    state.buffering.progressPercent = bufferingPercent;

    if (lifecycle != PlaybackLifecycleState::Empty
        && lifecycle != PlaybackLifecycleState::Closing) {
        state.generation = MediaGeneration{1};
        state.media.source = QStringLiteral("fixture://media");
    }

    if (lifecycle == PlaybackLifecycleState::Failed) {
        state.failure = PlaybackFailure{
            PlaybackFailureCategory::Media,
            -1,
            QStringLiteral("fixture failure")};
    }

    return PlaybackSnapshot{state};
}

void compareStatus(const PlayerStatusViewModel& viewModel, PlayerStatusKind expected)
{
    QCOMPARE(
        static_cast<int>(viewModel.status()),
        static_cast<int>(expected));
}

} // namespace

class PlayerStatusViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void emptyProjectsEmpty();
    void emptyToReadyPublishesNone();
    void loadingDominatesOpeningBuffering();
    void playingBufferingPublishesRoundedProgress();
    void pausedSuppressesBuffering();
    void endedProjectsEnded();
    void failedProjectsError();
    void irrelevantSnapshotDoesNotEmit();
};

void PlayerStatusViewModelTest::emptyProjectsEmpty()
{
    using namespace player::playback::domain;

    PlayerStatusViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Empty,
        PlaybackTransportState::Idle));

    compareStatus(viewModel, PlayerStatusKind::Empty);
    QCOMPARE(viewModel.statusKey(), QStringLiteral("empty"));
    QVERIFY(viewModel.visible());
    QVERIFY(!viewModel.errorVisible());
    QCOMPARE(viewModel.bufferingPercent(), -1);
}

void PlayerStatusViewModelTest::emptyToReadyPublishesNone()
{
    using namespace player::playback::domain;

    PlayerStatusViewModel viewModel;
    QSignalSpy spy(&viewModel, &PlayerStatusViewModel::stateChanged);

    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing));

    compareStatus(viewModel, PlayerStatusKind::None);
    QCOMPARE(viewModel.statusKey(), QString{});
    QVERIFY(!viewModel.visible());
    QVERIFY(!viewModel.errorVisible());
    QCOMPARE(viewModel.bufferingPercent(), -1);
    QCOMPARE(spy.count(), 1);
}

void PlayerStatusViewModelTest::loadingDominatesOpeningBuffering()
{
    using namespace player::playback::domain;

    PlayerStatusViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Opening,
        PlaybackTransportState::Idle,
        true,
        40.0));

    compareStatus(viewModel, PlayerStatusKind::Loading);
    QCOMPARE(viewModel.statusKey(), QStringLiteral("loading"));
    QVERIFY(viewModel.visible());
    QCOMPARE(viewModel.bufferingPercent(), -1);
}

void PlayerStatusViewModelTest::playingBufferingPublishesRoundedProgress()
{
    using namespace player::playback::domain;

    PlayerStatusViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing,
        true,
        52.6));

    compareStatus(viewModel, PlayerStatusKind::Buffering);
    QCOMPARE(viewModel.statusKey(), QStringLiteral("buffering"));
    QVERIFY(viewModel.visible());
    QCOMPARE(viewModel.bufferingPercent(), 53);
}

void PlayerStatusViewModelTest::pausedSuppressesBuffering()
{
    using namespace player::playback::domain;

    PlayerStatusViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Paused,
        true,
        80.0));

    compareStatus(viewModel, PlayerStatusKind::None);
    QVERIFY(!viewModel.visible());
    QCOMPARE(viewModel.bufferingPercent(), -1);
}

void PlayerStatusViewModelTest::endedProjectsEnded()
{
    using namespace player::playback::domain;

    PlayerStatusViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Ended,
        PlaybackTransportState::Stopped));

    compareStatus(viewModel, PlayerStatusKind::Ended);
    QCOMPARE(viewModel.statusKey(), QStringLiteral("ended"));
    QVERIFY(viewModel.visible());
    QVERIFY(!viewModel.errorVisible());
}

void PlayerStatusViewModelTest::failedProjectsError()
{
    using namespace player::playback::domain;

    PlayerStatusViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Failed,
        PlaybackTransportState::Stopped));

    compareStatus(viewModel, PlayerStatusKind::Error);
    QCOMPARE(viewModel.statusKey(), QStringLiteral("error"));
    QVERIFY(viewModel.visible());
    QVERIFY(viewModel.errorVisible());
}

void PlayerStatusViewModelTest::irrelevantSnapshotDoesNotEmit()
{
    using namespace player::playback::domain;

    PlayerStatusViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing));

    QSignalSpy spy(&viewModel, &PlayerStatusViewModel::stateChanged);

    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Idle));
    QCOMPARE(spy.count(), 0);

    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing));
    QCOMPARE(spy.count(), 0);

    viewModel.acceptSnapshot(makeSnapshot(
        PlaybackLifecycleState::Ready,
        PlaybackTransportState::Playing,
        true));
    QCOMPARE(spy.count(), 1);
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::PlayerStatusViewModelTest)
#include "player_status_view_model_test.moc"
