#include "tracks/application/audio_delay_controller.h"
#include "tracks/application/subtitle_delay_controller.h"

#include "playback/domain/state/playback_snapshot.h"

#include <QSignalSpy>
#include <QtTest>

#include <limits>

namespace player::tracks::application {
namespace {

using namespace player::playback::domain;

PlaybackSnapshot snapshotWithDelays(
    std::optional<double> subtitleDelay,
    std::optional<double> audioDelay)
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{7};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.controls.subtitleDelaySeconds = subtitleDelay;
    state.controls.audioDelaySeconds = audioDelay;
    return PlaybackSnapshot{std::move(state)};
}

} // namespace

class SubtitleDelayControllerTest final : public QObject
{
    Q_OBJECT

private slots:
    void unavailableWithoutBackendValue();
    void nudgesWithinFixedRange();
    void resetEmitsZero();
    void rejectsNonFiniteRequest();
    void pendingClearsWhenSnapshotConfirms();
    void delayTextFormatsSign();
};

void SubtitleDelayControllerTest::unavailableWithoutBackendValue()
{
    SubtitleDelayController controller;
    QVERIFY(!controller.canAdjust());
    QCOMPARE(controller.delaySeconds(), 0.0);

    QSignalSpy requestSpy(&controller, &SubtitleDelayController::subtitleDelayRequested);
    QVERIFY(!controller.nudgeLater());
    QVERIFY(!controller.nudgeEarlier());
    QVERIFY(!controller.reset());
    QCOMPARE(requestSpy.count(), 0);

    controller.acceptSnapshot(snapshotWithDelays(0.0, 0.0));
    QVERIFY(controller.canAdjust());
    QCOMPARE(controller.delaySeconds(), 0.0);
}

void SubtitleDelayControllerTest::nudgesWithinFixedRange()
{
    SubtitleDelayController controller;
    controller.acceptSnapshot(snapshotWithDelays(0.0, std::nullopt));

    QSignalSpy requestSpy(&controller, &SubtitleDelayController::subtitleDelayRequested);

    QVERIFY(controller.nudgeLater());
    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(requestSpy.first().at(0).toDouble(), 0.5);

    QVERIFY(controller.nudgeLater());
    QCOMPARE(requestSpy.count(), 2);
    QCOMPARE(requestSpy.at(1).at(0).toDouble(), 1.0);

    QVERIFY(controller.nudgeEarlier());
    QCOMPARE(requestSpy.count(), 3);
    QCOMPARE(requestSpy.at(2).at(0).toDouble(), 0.5);

    // Clamped at the upper bound: value unchanged, no redundant command.
    controller.acceptSnapshot(snapshotWithDelays(10.0, std::nullopt));
    const int beforeClamp = requestSpy.count();
    QVERIFY(controller.nudgeLater());
    QCOMPARE(requestSpy.count(), beforeClamp);
    QCOMPARE(controller.delaySeconds(), 10.0);

    // Clamped at the lower bound: value unchanged, no redundant command.
    controller.acceptSnapshot(snapshotWithDelays(-10.0, std::nullopt));
    const int beforeLowerClamp = requestSpy.count();
    QVERIFY(controller.nudgeEarlier());
    QCOMPARE(requestSpy.count(), beforeLowerClamp);
    QCOMPARE(controller.delaySeconds(), -10.0);
}

void SubtitleDelayControllerTest::resetEmitsZero()
{
    SubtitleDelayController controller;
    controller.acceptSnapshot(snapshotWithDelays(3.5, std::nullopt));

    QSignalSpy requestSpy(&controller, &SubtitleDelayController::subtitleDelayRequested);
    QVERIFY(controller.reset());
    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(requestSpy.first().at(0).toDouble(), 0.0);
}

void SubtitleDelayControllerTest::rejectsNonFiniteRequest()
{
    SubtitleDelayController controller;
    controller.acceptSnapshot(snapshotWithDelays(0.0, std::nullopt));

    QSignalSpy requestSpy(&controller, &SubtitleDelayController::subtitleDelayRequested);
    QVERIFY(!controller.requestSet(
        std::numeric_limits<double>::quiet_NaN()));
    QCOMPARE(requestSpy.count(), 0);
}

void SubtitleDelayControllerTest::pendingClearsWhenSnapshotConfirms()
{
    SubtitleDelayController controller;
    controller.acceptSnapshot(snapshotWithDelays(0.0, std::nullopt));

    QSignalSpy requestSpy(&controller, &SubtitleDelayController::subtitleDelayRequested);
    QVERIFY(controller.requestSet(1.5));
    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(controller.delaySeconds(), 1.5);

    // Backend confirms the pending value; the pending state clears.
    controller.acceptSnapshot(snapshotWithDelays(1.5, std::nullopt));
    QCOMPARE(controller.delaySeconds(), 1.5);

    // Backend resets (e.g. media switch); controller follows.
    controller.acceptSnapshot(PlaybackSnapshot::stopped(MediaGeneration{8}));
    QVERIFY(!controller.canAdjust());
    QCOMPARE(controller.delaySeconds(), 0.0);
}

void SubtitleDelayControllerTest::delayTextFormatsSign()
{
    SubtitleDelayController controller;
    controller.acceptSnapshot(snapshotWithDelays(0.0, std::nullopt));
    QCOMPARE(controller.delayText(), QStringLiteral("0.0s"));

    controller.acceptSnapshot(snapshotWithDelays(0.5, std::nullopt));
    QCOMPARE(controller.delayText(), QStringLiteral("+0.5s"));

    controller.acceptSnapshot(snapshotWithDelays(-1.5, std::nullopt));
    QCOMPARE(controller.delayText(), QStringLiteral("-1.5s"));
}

class AudioDelayControllerTest final : public QObject
{
    Q_OBJECT

private slots:
    void staysIndependentFromSubtitleDelay();
    void nudgesAndResetsWithinRange();
};

void AudioDelayControllerTest::staysIndependentFromSubtitleDelay()
{
    AudioDelayController audio;
    SubtitleDelayController subtitle;

    // Subtitle delay present, audio delay absent: only subtitle can adjust.
    audio.acceptSnapshot(snapshotWithDelays(2.0, std::nullopt));
    subtitle.acceptSnapshot(snapshotWithDelays(2.0, std::nullopt));
    QVERIFY(!audio.canAdjust());
    QVERIFY(subtitle.canAdjust());

    // Audio delay present: audio can adjust and shows its own value.
    audio.acceptSnapshot(snapshotWithDelays(2.0, -1.0));
    QVERIFY(audio.canAdjust());
    QCOMPARE(audio.delaySeconds(), -1.0);
    QCOMPARE(subtitle.delaySeconds(), 2.0);
}

void AudioDelayControllerTest::nudgesAndResetsWithinRange()
{
    AudioDelayController controller;
    controller.acceptSnapshot(snapshotWithDelays(std::nullopt, 0.0));

    QSignalSpy requestSpy(&controller, &AudioDelayController::audioDelayRequested);

    QVERIFY(controller.nudgeEarlier());
    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(requestSpy.first().at(0).toDouble(), -0.5);

    QVERIFY(controller.reset());
    QCOMPARE(requestSpy.count(), 2);
    QCOMPARE(requestSpy.at(1).at(0).toDouble(), 0.0);

    QCOMPARE(controller.minimumSeconds(), -10.0);
    QCOMPARE(controller.maximumSeconds(), 10.0);
    QCOMPARE(controller.stepSeconds(), 0.5);
}

} // namespace player::tracks::application

QTEST_GUILESS_MAIN(player::tracks::application::SubtitleDelayControllerTest)
#include "delay_controller_test.moc"
