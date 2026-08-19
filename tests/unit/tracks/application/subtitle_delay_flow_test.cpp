#include "foundation/ids/request_id.h"
#include "playback/application/requests/request_tracker.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/commands/subtitle_delay_command.h"
#include "playback/domain/events/playback_event.h"
#include "playback/domain/state/playback_reducer.h"
#include "playback/domain/state/playback_snapshot.h"
#include "playback/infrastructure/mpv/commands/mpv_command_encoder.h"
#include "playback/infrastructure/mpv/commands/mpv_playback_command_mapper.h"
#include "playback/infrastructure/mpv/events/mpv_event.h"
#include "playback/infrastructure/mpv/events/mpv_playback_event_mapper.h"
#include "playback/infrastructure/mpv/properties/mpv_property_change.h"
#include "tracks/application/subtitle_delay_controller.h"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QtTest>

#include <cmath>
#include <limits>

namespace player::tracks::application {
namespace {

using namespace player::playback;

QString readSource(const QString& relativePath)
{
    QFile file(QDir(QStringLiteral(PLAYER_SOURCE_DIR)).filePath(relativePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

domain::PlaybackSnapshot readySnapshot(
    domain::MediaGeneration generation,
    double subtitleDelaySeconds,
    bool subtitleSelected = true)
{
    domain::PlaybackSnapshotState state;
    state.generation = generation;
    state.lifecycle = domain::PlaybackLifecycleState::Ready;
    if (subtitleSelected) {
        state.tracks.selectedSubtitleId = qint64{1};
    }
    state.controls.subtitleDelaySeconds = subtitleDelaySeconds;
    return domain::PlaybackSnapshot{std::move(state)};
}

} // namespace

class SubtitleDelayFlowTest final : public QObject
{
    Q_OBJECT

private slots:
    void controllerHandlesPositiveNegativeAndReset();
    void controllerRejectsInvalidAndClearsStalePending();
    void commandMapsToGenerationScopedMpvPropertySet();
    void propertyObservationOwnsConfirmedSnapshotValue();
    void sourceChainKeepsQmlAwayFromMpv();
};

void SubtitleDelayFlowTest::controllerHandlesPositiveNegativeAndReset()
{
    QList<double> submissions;
    SubtitleDelayController controller(
        [&submissions](const domain::SetSubtitleDelayCommand& command) {
            submissions.append(command.seconds);
            return true;
        });
    QSignalSpy confirmedSpy(&controller, &SubtitleDelayController::delayConfirmed);

    const domain::MediaGeneration generation{7};
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

    QVERIFY(controller.setDelaySeconds(-0.10));
    QVERIFY(controller.pending());
    QCOMPARE(controller.pendingTargetMilliseconds(), -100);
    controller.acceptSnapshot(readySnapshot(generation, -0.10));
    QVERIFY(!controller.pending());
    QCOMPARE(controller.delayMilliseconds(), -100);

    QVERIFY(controller.resetDelay());
    QVERIFY(controller.pending());
    QCOMPARE(controller.pendingTargetMilliseconds(), 0);
    controller.acceptSnapshot(readySnapshot(generation, 0.0));
    QVERIFY(!controller.pending());
    QCOMPARE(controller.delayMilliseconds(), 0);
    QCOMPARE(submissions.size(), 3);
}

void SubtitleDelayFlowTest::controllerRejectsInvalidAndClearsStalePending()
{
    QList<double> submissions;
    SubtitleDelayController controller(
        [&submissions](const domain::SetSubtitleDelayCommand& command) {
            submissions.append(command.seconds);
            return true;
        });

    const domain::MediaGeneration generation{11};
    controller.acceptSnapshot(readySnapshot(generation, 0.0));

    QVERIFY(!controller.setDelaySeconds(std::numeric_limits<double>::quiet_NaN()));
    QVERIFY(!controller.setDelaySeconds(2.1));
    QVERIFY(!controller.setDelaySeconds(-2.1));
    QCOMPARE(submissions.size(), 0);

    QVERIFY(controller.setDelaySeconds(0.27));
    QCOMPARE(submissions.size(), 1);
    QVERIFY(std::abs(submissions.at(0) - 0.25) < 0.0001);
    QCOMPARE(controller.pendingTargetMilliseconds(), 250);

    controller.acceptSnapshot(readySnapshot(domain::MediaGeneration{12}, 0.0));
    QVERIFY(!controller.pending());

    controller.acceptSnapshot(readySnapshot(domain::MediaGeneration{12}, 0.0, false));
    QVERIFY(!controller.available());
    QVERIFY(!controller.setDelaySeconds(0.1));

    SubtitleDelayController rejectedController(
        [](const domain::SetSubtitleDelayCommand&) { return false; });
    rejectedController.acceptSnapshot(readySnapshot(domain::MediaGeneration{13}, 0.0));
    QVERIFY(!rejectedController.setDelaySeconds(0.5));
    QVERIFY(!rejectedController.pending());
}

void SubtitleDelayFlowTest::commandMapsToGenerationScopedMpvPropertySet()
{
    const domain::PlaybackCommand command{
        player::ids::RequestId{81},
        domain::PlaybackCommandPayload{domain::SetSubtitleDelayCommand{0.25}}};
    QVERIFY(!domain::validatePlaybackCommand(command).has_value());

    const auto mapped = mpv::MpvPlaybackCommandMapper::map(command.payload());
    QVERIFY(mapped.has_value());

    QString error;
    const auto encoded = mpv::MpvCommandEncoder::encode(*mapped, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{
            QByteArrayLiteral("set"),
            QByteArrayLiteral("sub-delay"),
            QByteArrayLiteral("0.25")}));

    const domain::PlaybackCommand invalidHigh{
        player::ids::RequestId{82},
        domain::PlaybackCommandPayload{domain::SetSubtitleDelayCommand{2.1}}};
    const auto validationError = domain::validatePlaybackCommand(invalidHigh);
    QVERIFY(validationError.has_value());
    QVERIFY(*validationError == domain::PlaybackCommandValidationError::InvalidSubtitleDelay);

    application::RequestTracker tracker;
    const domain::MediaGeneration generation{22};
    QCOMPARE(
        tracker.track(command, generation),
        application::RequestTrackStatus::Tracked);
    const auto record = tracker.record(command.requestId());
    QVERIFY(record.has_value());
    QVERIFY(record->type == application::PlaybackRequestType::SetSubtitleDelay);
    QVERIFY(record->generation.has_value());
    QCOMPARE(record->generation->value(), generation.value());

    const domain::PlaybackCommand replacement{
        player::ids::RequestId{83},
        domain::PlaybackCommandPayload{domain::SetSubtitleDelayCommand{-0.25}}};
    QCOMPARE(
        tracker.track(replacement, generation),
        application::RequestTrackStatus::Tracked);
    QCOMPARE(tracker.supersedePendingFor(replacement, generation), std::size_t{1});
}

void SubtitleDelayFlowTest::propertyObservationOwnsConfirmedSnapshotValue()
{
    mpv::MpvEvent event;
    event.type = mpv::MpvEventType::PropertyChange;
    event.payload = mpv::MpvPropertyChange{
        mpv::MpvPropertyId::SubtitleDelay,
        mpv::MpvPropertyValue{0.25}};

    auto mapped = mpv::MpvPlaybackEventMapper::map(event);
    QVERIFY(mapped.has_value());
    const auto* delay = std::get_if<domain::SubtitleDelayChangedEvent>(&mapped->payload);
    QVERIFY(delay != nullptr);
    QVERIFY(delay->seconds.has_value());
    QVERIFY(std::abs(*delay->seconds - 0.25) < 0.0001);

    domain::PlaybackSnapshotState state;
    state.generation = domain::MediaGeneration{31};
    state.lifecycle = domain::PlaybackLifecycleState::Ready;
    state.tracks.selectedSubtitleId = qint64{1};
    mapped->generation = state.generation;

    const domain::PlaybackSnapshot reduced = domain::reducePlaybackSnapshot(
        domain::PlaybackSnapshot{std::move(state)},
        *mapped);
    QVERIFY(reduced.controls().subtitleDelaySeconds.has_value());
    QVERIFY(std::abs(*reduced.controls().subtitleDelaySeconds - 0.25) < 0.0001);

    const domain::PlaybackSnapshot opening = domain::reducePlaybackSnapshot(
        reduced,
        domain::PlaybackEvent{domain::MediaLoadStartedEvent{}});
    QVERIFY(!opening.controls().subtitleDelaySeconds.has_value());
}

void SubtitleDelayFlowTest::sourceChainKeepsQmlAwayFromMpv()
{
    const QString controller = readSource(
        QStringLiteral("src/tracks/application/subtitle_delay_controller.cpp"));
    const QString backend = readSource(
        QStringLiteral("src/playback/application/session/backend/playback_session_backend.cpp"));
    const QString session = readSource(
        QStringLiteral("src/playback/application/session/playback_session.cpp"));
    const QString control = readSource(
        QStringLiteral("src/presentation/qml/features/tracks/SubtitleDelayControl.qml"));
    const QString popup = readSource(
        QStringLiteral("src/presentation/qml/features/tracks/TrackSelectionPopup.qml"));
    const QString screen = readSource(
        QStringLiteral("src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString shell = readSource(
        QStringLiteral("src/presentation/qml/shell/MainWindow.qml"));
    const QString hud = readSource(
        QStringLiteral("src/presentation/qml/screens/player/overlays/hud/PlayerHudOverlay.qml"));

    QVERIFY(!controller.isEmpty());
    QVERIFY(!backend.isEmpty());
    QVERIFY(!session.isEmpty());
    QVERIFY(!control.isEmpty());
    QVERIFY(!popup.isEmpty());
    QVERIFY(!screen.isEmpty());
    QVERIFY(!shell.isEmpty());
    QVERIFY(!hud.isEmpty());

    QVERIFY(controller.contains(QStringLiteral("pendingTargetSeconds_")));
    QVERIFY(controller.contains(QStringLiteral("snapshot.controls().subtitleDelaySeconds")));
    QVERIFY(backend.contains(QStringLiteral("MpvPropertyId::SubtitleDelay")));
    QVERIFY(session.contains(QStringLiteral("PlaybackRequestType::SetSubtitleDelay")));
    QVERIFY(session.contains(QStringLiteral("refreshSubtitleDelayState")));

    QVERIFY(control.contains(QStringLiteral("objectName: \"subtitleDelaySlider\"")));
    QVERIFY(control.contains(QStringLiteral("root.controller.resetDelay()")));
    QVERIFY(control.contains(QStringLiteral("root.controller.pending")));
    QVERIFY(!control.contains(QStringLiteral("mpv_")));
    QVERIFY(!control.contains(QStringLiteral("mpv_command")));

    QVERIFY(popup.contains(QStringLiteral("SubtitleDelayControl")));
    QVERIFY(popup.contains(QStringLiteral("subtitleDelayController")));
    QVERIFY(screen.contains(QStringLiteral("subtitleDelayController")));
    QVERIFY(shell.contains(QStringLiteral("subtitleDelayController")));
    QVERIFY(hud.contains(QStringLiteral("case \"subtitleDelay\"")));
}

} // namespace player::tracks::application

QTEST_GUILESS_MAIN(player::tracks::application::SubtitleDelayFlowTest)
#include "subtitle_delay_flow_test.moc"
