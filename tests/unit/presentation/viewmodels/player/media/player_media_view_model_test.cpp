#include "presentation/viewmodels/player/media/player_media_view_model.h"

#include "playback/domain/models/video_stream_info.h"
#include "playback/domain/state/media_generation.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QSignalSpy>
#include <QtTest>

namespace player::presentation {
namespace {

player::playback::domain::PlaybackSnapshot makeSnapshot(
    player::playback::domain::PlaybackLifecycleState lifecycle,
    bool withVideo = false)
{
    using namespace player::playback::domain;

    PlaybackSnapshotState state;
    state.lifecycle = lifecycle;
    if (lifecycle != PlaybackLifecycleState::Empty
        && lifecycle != PlaybackLifecycleState::Closing) {
        state.generation = MediaGeneration{1};
        state.media.source = QStringLiteral("fixture://media");
    }
    if (withVideo) {
        state.streams.video = VideoStreamInfo{};
    }
    return PlaybackSnapshot{state};
}

} // namespace

class PlayerMediaViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void emptyStartsWithoutMedia();
    void openingDoesNotExposeUnestablishedMedia();
    void readyWithoutSourceDoesNotExposeMedia();
    void readyAudioProjectsMediaWithoutVideo();
    void readyVideoProjectsVisibleVideo();
    void endedKeepsMediaProjection();
    void failedClearsViewportProjection();
    void unchangedProjectionDoesNotEmit();
};

void PlayerMediaViewModelTest::emptyStartsWithoutMedia()
{
    PlayerMediaViewModel viewModel;
    QVERIFY(!viewModel.hasMedia());
    QVERIFY(!viewModel.hasVideo());
}

void PlayerMediaViewModelTest::openingDoesNotExposeUnestablishedMedia()
{
    using player::playback::domain::PlaybackLifecycleState;

    PlayerMediaViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Opening, true));
    QVERIFY(!viewModel.hasMedia());
    QVERIFY(!viewModel.hasVideo());
}

void PlayerMediaViewModelTest::readyWithoutSourceDoesNotExposeMedia()
{
    using namespace player::playback::domain;

    PlaybackSnapshotState state;
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.generation = MediaGeneration{1};
    state.streams.video = VideoStreamInfo{};

    PlayerMediaViewModel viewModel;
    viewModel.acceptSnapshot(PlaybackSnapshot{state});
    QVERIFY(!viewModel.hasMedia());
    QVERIFY(!viewModel.hasVideo());
}

void PlayerMediaViewModelTest::readyAudioProjectsMediaWithoutVideo()
{
    using player::playback::domain::PlaybackLifecycleState;

    PlayerMediaViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Ready));
    QVERIFY(viewModel.hasMedia());
    QVERIFY(!viewModel.hasVideo());
}

void PlayerMediaViewModelTest::readyVideoProjectsVisibleVideo()
{
    using player::playback::domain::PlaybackLifecycleState;

    PlayerMediaViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Ready, true));
    QVERIFY(viewModel.hasMedia());
    QVERIFY(viewModel.hasVideo());
}

void PlayerMediaViewModelTest::endedKeepsMediaProjection()
{
    using player::playback::domain::PlaybackLifecycleState;

    PlayerMediaViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Ended, true));
    QVERIFY(viewModel.hasMedia());
    QVERIFY(viewModel.hasVideo());
}

void PlayerMediaViewModelTest::failedClearsViewportProjection()
{
    using player::playback::domain::PlaybackLifecycleState;

    PlayerMediaViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Ready, true));
    QVERIFY(viewModel.hasVideo());

    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Failed, true));
    QVERIFY(!viewModel.hasMedia());
    QVERIFY(!viewModel.hasVideo());
}

void PlayerMediaViewModelTest::unchangedProjectionDoesNotEmit()
{
    using player::playback::domain::PlaybackLifecycleState;

    PlayerMediaViewModel viewModel;
    QSignalSpy spy(&viewModel, &PlayerMediaViewModel::stateChanged);

    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Empty));
    QCOMPARE(spy.count(), 0);

    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Ready));
    QCOMPARE(spy.count(), 1);

    viewModel.acceptSnapshot(makeSnapshot(PlaybackLifecycleState::Ready));
    QCOMPARE(spy.count(), 1);
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::PlayerMediaViewModelTest)
#include "player_media_view_model_test.moc"
