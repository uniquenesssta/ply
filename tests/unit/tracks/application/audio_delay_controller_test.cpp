#include "playback/domain/commands/audio_delay_command.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"
#include "tracks/application/audio_delay_controller.h"

#include <QList>
#include <QSignalSpy>
#include <QtTest>

#include <cmath>
#include <limits>
#include <utility>

namespace player::tracks::application {
namespace {

using namespace player::playback::domain;

PlaybackSnapshot readySnapshot(
    MediaGeneration generation,
    double audioDelaySeconds,
    bool audioSelected = true,
    double subtitleDelaySeconds = 0.0)
{
    PlaybackSnapshotState state;
    state.generation = generation;
    state.lifecycle = PlaybackLifecycleState::Ready;
    if (audioSelected) {
        state.tracks.selectedAudioId = qint64{1};
    }
    state.controls.audioDelaySeconds = audioDelaySeconds;
    state.controls.subtitleDelaySeconds = subtitleDelaySeconds;
    return PlaybackSnapshot{std::move(state)};
}

} // namespace

class AudioDelayControllerTest final : public QObject
{
    Q_OBJECT

private slots:
    void handlesPositiveNegativeAndReset();
    void quantizesRejectsInvalidAndDoesNotTouchSubtitleState();
    void clearsPendingAcrossGenerationAndFailure();
    void requiresSelectedAudioAndConfirmedBackendValue();
};

void AudioDelayControllerTest::handlesPositiveNegativeAndReset()
{
    QList<double> submissions;
    AudioDelayController controller(
        [&submissions](const SetAudioDelayCommand& command) {
            submissions.append(command.seconds);
            return true;
        });
    QSignalSpy confirmedSpy(&controller, &AudioDelayController::delayConfirmed);

    const MediaGeneration generation{101};
    controller.acceptSnapshot(readySnapshot(generation, 0.0));
    QVERIFY(controller.available());
    QVERIFY(!controller.pending());
    QCOMPARE(controller.delayMilliseconds(), 0);

    QVERIFY(controller.setDelaySeconds(0.25));
    QVERIFY(controller.pending());
    QCOMPARE(controller.pendingTargetMilliseconds(), 250);
    QCOMPARE(controller.delayMilliseconds(), 0);
    QCOMPARE(submissions.size(), 1);
    QVERIFY(std::abs(submissions.at(0) - 0.25) < 0.0001);

    controller.acceptSnapshot(readySnapshot(generation, 0.25));
    QVERIFY(!controller.pending());
    QCOMPARE(controller.delayMilliseconds(), 250);
    QCOMPARE(confirmedSpy.size(), 1);
    QCOMPARE(confirmedSpy.at(0).at(0).toInt(), 250);

    QVERIFY(controller.setDelaySeconds(-0.25));
    QCOMPARE(controller.pendingTargetMilliseconds(), -250);
    controller.acceptSnapshot(readySnapshot(generation, -0.25));
    QVERIFY(!controller.pending());
    QCOMPARE(controller.delayMilliseconds(), -250);

    QVERIFY(controller.resetDelay());
    QCOMPARE(controller.pendingTargetMilliseconds(), 0);
    controller.acceptSnapshot(readySnapshot(generation, 0.0));
    QCOMPARE(controller.delayMilliseconds(), 0);
    QCOMPARE(submissions.size(), 3);
}

void AudioDelayControllerTest::quantizesRejectsInvalidAndDoesNotTouchSubtitleState()
{
    QList<double> submissions;
    AudioDelayController controller(
        [&submissions](const SetAudioDelayCommand& command) {
            submissions.append(command.seconds);
            return true;
        });

    const MediaGeneration generation{102};
    controller.acceptSnapshot(readySnapshot(generation, 0.0, true, -0.4));

    QVERIFY(!controller.setDelaySeconds(std::numeric_limits<double>::quiet_NaN()));
    QVERIFY(!controller.setDelaySeconds(2.1));
    QVERIFY(!controller.setDelaySeconds(-2.1));
    QCOMPARE(submissions.size(), 0);

    QVERIFY(controller.setDelaySeconds(0.27));
    QCOMPARE(submissions.size(), 1);
    QVERIFY(std::abs(submissions.at(0) - 0.25) < 0.0001);
    QCOMPARE(controller.pendingTargetMilliseconds(), 250);

    controller.acceptSnapshot(readySnapshot(generation, 0.25, true, 1.5));
    QCOMPARE(controller.delayMilliseconds(), 250);
    QVERIFY(!controller.pending());
}

void AudioDelayControllerTest::clearsPendingAcrossGenerationAndFailure()
{
    AudioDelayController controller(
        [](const SetAudioDelayCommand&) { return true; });

    controller.acceptSnapshot(readySnapshot(MediaGeneration{103}, 0.0));
    QVERIFY(controller.setDelaySeconds(0.5));
    QVERIFY(controller.pending());

    controller.acceptSnapshot(readySnapshot(MediaGeneration{104}, 0.0));
    QVERIFY(!controller.pending());

    QVERIFY(controller.setDelaySeconds(-0.5));
    QVERIFY(controller.pending());
    QVERIFY(controller.rejectPendingDelay());
    QVERIFY(!controller.pending());
    QVERIFY(!controller.rejectPendingDelay());

    AudioDelayController rejectedController(
        [](const SetAudioDelayCommand&) { return false; });
    rejectedController.acceptSnapshot(readySnapshot(MediaGeneration{105}, 0.0));
    QVERIFY(!rejectedController.setDelaySeconds(0.5));
    QVERIFY(!rejectedController.pending());
}

void AudioDelayControllerTest::requiresSelectedAudioAndConfirmedBackendValue()
{
    AudioDelayController controller(
        [](const SetAudioDelayCommand&) { return true; });

    controller.acceptSnapshot(readySnapshot(MediaGeneration{106}, 0.0, false));
    QVERIFY(!controller.available());
    QVERIFY(!controller.setDelaySeconds(0.25));

    PlaybackSnapshotState unavailableState;
    unavailableState.generation = MediaGeneration{106};
    unavailableState.lifecycle = PlaybackLifecycleState::Ready;
    unavailableState.tracks.selectedAudioId = qint64{1};
    controller.acceptSnapshot(PlaybackSnapshot{std::move(unavailableState)});
    QVERIFY(!controller.available());

    controller.acceptSnapshot(readySnapshot(MediaGeneration{106}, 0.0));
    QVERIFY(controller.available());
}

} // namespace player::tracks::application

QTEST_GUILESS_MAIN(player::tracks::application::AudioDelayControllerTest)
#include "audio_delay_controller_test.moc"
