#include "presentation/viewmodels/player/volume/player_volume_view_model.h"

#include <QSignalSpy>
#include <QtTest>

#include <cmath>
#include <optional>

namespace player::presentation {
namespace {

using player::playback::domain::PlaybackSnapshot;
using player::playback::domain::PlaybackSnapshotState;

bool fuzzyEqual(double left, double right)
{
    return std::abs(left - right) <= 0.000001;
}

PlaybackSnapshot volumeSnapshot(
    std::optional<double> percent,
    std::optional<bool> muted)
{
    PlaybackSnapshotState state;
    state.controls.volumePercent = percent;
    state.controls.muted = muted;
    return PlaybackSnapshot{state};
}

} // namespace

class PlayerVolumeViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void missingBackendStateDisablesControls();
    void snapshotProjectsVolumeAndMute();
    void volumePreviewHoldsLatestTargetUntilAcknowledged();
    void rejectedVolumeSubmissionRestoresBackendValue();
    void muteToggleWaitsForBackendAcknowledgement();
    void missingBackendStateClearsPendingRequests();
};

void PlayerVolumeViewModelTest::missingBackendStateDisablesControls()
{
    PlayerVolumeViewModel viewModel;

    QVERIFY(!viewModel.canAdjustVolume());
    QVERIFY(!viewModel.canToggleMute());
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.0));
    QVERIFY(!viewModel.muted());
    QVERIFY(!viewModel.requestVolumeNormalized(0.5));
    QVERIFY(!viewModel.requestToggleMuted());
}

void PlayerVolumeViewModelTest::snapshotProjectsVolumeAndMute()
{
    PlayerVolumeViewModel viewModel;

    viewModel.acceptSnapshot(volumeSnapshot(64.0, false));

    QVERIFY(viewModel.canAdjustVolume());
    QVERIFY(viewModel.canToggleMute());
    QVERIFY(fuzzyEqual(viewModel.volumePercent(), 64.0));
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.64));
    QVERIFY(!viewModel.muted());
    QVERIFY(!viewModel.volumePending());
    QVERIFY(!viewModel.mutePending());
}

void PlayerVolumeViewModelTest::volumePreviewHoldsLatestTargetUntilAcknowledged()
{
    PlayerVolumeViewModel viewModel;
    QSignalSpy volumeSpy(&viewModel, &PlayerVolumeViewModel::volumeRequested);

    viewModel.acceptSnapshot(volumeSnapshot(64.0, false));
    QVERIFY(viewModel.requestVolumeNormalized(0.8));
    QVERIFY(viewModel.volumePending());
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.8));
    QCOMPARE(volumeSpy.count(), 1);
    QVERIFY(fuzzyEqual(volumeSpy.at(0).at(0).toDouble(), 80.0));

    viewModel.acceptSnapshot(volumeSnapshot(65.0, false));
    QVERIFY(viewModel.volumePending());
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.8));

    QVERIFY(viewModel.requestVolumeNormalized(0.9));
    QCOMPARE(volumeSpy.count(), 2);
    QVERIFY(fuzzyEqual(volumeSpy.at(1).at(0).toDouble(), 90.0));
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.9));

    viewModel.acceptSnapshot(volumeSnapshot(80.0, false));
    QVERIFY(viewModel.volumePending());
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.9));

    viewModel.acceptSnapshot(volumeSnapshot(89.7, false));
    QVERIFY(!viewModel.volumePending());
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.897));
}

void PlayerVolumeViewModelTest::rejectedVolumeSubmissionRestoresBackendValue()
{
    PlayerVolumeViewModel viewModel;

    viewModel.acceptSnapshot(volumeSnapshot(35.0, false));
    QVERIFY(viewModel.requestVolumeNormalized(0.7));
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.7));
    QVERIFY(viewModel.rejectPendingVolume());
    QVERIFY(!viewModel.volumePending());
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.35));
}

void PlayerVolumeViewModelTest::muteToggleWaitsForBackendAcknowledgement()
{
    PlayerVolumeViewModel viewModel;
    QSignalSpy muteSpy(&viewModel, &PlayerVolumeViewModel::mutedRequested);

    viewModel.acceptSnapshot(volumeSnapshot(50.0, false));
    QVERIFY(viewModel.requestToggleMuted());
    QVERIFY(viewModel.mutePending());
    QVERIFY(viewModel.muted());
    QCOMPARE(muteSpy.count(), 1);
    QCOMPARE(muteSpy.at(0).at(0).toBool(), true);

    QVERIFY(!viewModel.requestToggleMuted());
    QCOMPARE(muteSpy.count(), 1);

    viewModel.acceptSnapshot(volumeSnapshot(50.0, false));
    QVERIFY(viewModel.mutePending());
    QVERIFY(viewModel.muted());

    viewModel.acceptSnapshot(volumeSnapshot(50.0, true));
    QVERIFY(!viewModel.mutePending());
    QVERIFY(viewModel.muted());

    QVERIFY(viewModel.requestToggleMuted());
    QCOMPARE(muteSpy.count(), 2);
    QCOMPARE(muteSpy.at(1).at(0).toBool(), false);
    QVERIFY(viewModel.mutePending());
    QVERIFY(!viewModel.muted());
}

void PlayerVolumeViewModelTest::missingBackendStateClearsPendingRequests()
{
    PlayerVolumeViewModel viewModel;

    viewModel.acceptSnapshot(volumeSnapshot(75.0, false));
    QVERIFY(viewModel.requestVolumeNormalized(0.25));
    QVERIFY(viewModel.requestToggleMuted());
    QVERIFY(viewModel.volumePending());
    QVERIFY(viewModel.mutePending());

    viewModel.acceptSnapshot(volumeSnapshot(std::nullopt, std::nullopt));
    QVERIFY(!viewModel.canAdjustVolume());
    QVERIFY(!viewModel.canToggleMute());
    QVERIFY(!viewModel.volumePending());
    QVERIFY(!viewModel.mutePending());
    QVERIFY(fuzzyEqual(viewModel.normalizedVolume(), 0.0));
    QVERIFY(!viewModel.muted());
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::PlayerVolumeViewModelTest)
#include "player_volume_view_model_test.moc"
