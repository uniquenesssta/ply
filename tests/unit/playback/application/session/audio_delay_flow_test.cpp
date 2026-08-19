#include "foundation/ids/request_id.h"
#include "playback/application/requests/request_tracker.h"
#include "playback/domain/commands/audio_delay_command.h"
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

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QList>
#include <QString>
#include <QtTest>

#include <cmath>
#include <limits>
#include <optional>
#include <variant>

namespace player::playback::application {
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

} // namespace

class AudioDelayFlowTest final : public QObject
{
    Q_OBJECT

private slots:
    void commandUsesIndependentGenerationScopedRequestLane();
    void rejectsOutOfRangeAndNonFiniteValues();
    void propertyObservationOwnsAudioSnapshotWithoutTouchingSubtitle();
    void sourceChainRefreshesConfirmedAudioDelay();
};

void AudioDelayFlowTest::commandUsesIndependentGenerationScopedRequestLane()
{
    const domain::PlaybackCommand firstAudio{
        player::ids::RequestId{701},
        domain::PlaybackCommandPayload{domain::SetAudioDelayCommand{0.25}}};
    QVERIFY(!domain::validatePlaybackCommand(firstAudio).has_value());

    const auto mapped = mpv::MpvPlaybackCommandMapper::map(firstAudio.payload());
    QVERIFY(mapped.has_value());

    QString error;
    const auto encoded = mpv::MpvCommandEncoder::encode(*mapped, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{
            QByteArrayLiteral("set"),
            QByteArrayLiteral("audio-delay"),
            QByteArrayLiteral("0.25")}));

    const domain::MediaGeneration generation{61};
    RequestTracker tracker;
    QCOMPARE(tracker.track(firstAudio, generation), RequestTrackStatus::Tracked);

    const domain::PlaybackCommand subtitle{
        player::ids::RequestId{702},
        domain::PlaybackCommandPayload{domain::SetSubtitleDelayCommand{0.10}}};
    QCOMPARE(tracker.track(subtitle, generation), RequestTrackStatus::Tracked);

    const domain::PlaybackCommand replacementAudio{
        player::ids::RequestId{703},
        domain::PlaybackCommandPayload{domain::SetAudioDelayCommand{-0.25}}};
    QCOMPARE(tracker.track(replacementAudio, generation), RequestTrackStatus::Tracked);
    QCOMPARE(tracker.supersedePendingFor(replacementAudio, generation), std::size_t{1});

    const auto firstRecord = tracker.record(firstAudio.requestId());
    const auto subtitleRecord = tracker.record(subtitle.requestId());
    const auto replacementRecord = tracker.record(replacementAudio.requestId());
    QVERIFY(firstRecord.has_value());
    QVERIFY(subtitleRecord.has_value());
    QVERIFY(replacementRecord.has_value());
    QCOMPARE(firstRecord->type, PlaybackRequestType::SetAudioDelay);
    QCOMPARE(firstRecord->state, PlaybackRequestState::Cancelled);
    QCOMPARE(firstRecord->cancellationReason, PlaybackRequestCancellationReason::Superseded);
    QCOMPARE(subtitleRecord->type, PlaybackRequestType::SetSubtitleDelay);
    QCOMPARE(subtitleRecord->state, PlaybackRequestState::Pending);
    QCOMPARE(replacementRecord->type, PlaybackRequestType::SetAudioDelay);
    QCOMPARE(replacementRecord->state, PlaybackRequestState::Pending);
    QVERIFY(replacementRecord->generation.has_value());
    QCOMPARE(replacementRecord->generation->value(), generation.value());
}

void AudioDelayFlowTest::rejectsOutOfRangeAndNonFiniteValues()
{
    const domain::PlaybackCommand tooHigh{
        player::ids::RequestId{711},
        domain::PlaybackCommandPayload{domain::SetAudioDelayCommand{2.01}}};
    QCOMPARE(
        domain::validatePlaybackCommand(tooHigh),
        std::optional{domain::PlaybackCommandValidationError::InvalidAudioDelay});

    const domain::PlaybackCommand tooLow{
        player::ids::RequestId{712},
        domain::PlaybackCommandPayload{domain::SetAudioDelayCommand{-2.01}}};
    QCOMPARE(
        domain::validatePlaybackCommand(tooLow),
        std::optional{domain::PlaybackCommandValidationError::InvalidAudioDelay});

    const domain::PlaybackCommand notANumber{
        player::ids::RequestId{713},
        domain::PlaybackCommandPayload{domain::SetAudioDelayCommand{
            std::numeric_limits<double>::quiet_NaN()}}};
    QCOMPARE(
        domain::validatePlaybackCommand(notANumber),
        std::optional{domain::PlaybackCommandValidationError::InvalidAudioDelay});

    QCOMPARE(domain::kAudioDelayMinimumSeconds, -2.0);
    QCOMPARE(domain::kAudioDelayMaximumSeconds, 2.0);
    QCOMPARE(domain::kAudioDelayStepSeconds, 0.05);

    QString error;
    const auto invalidEncoded = mpv::MpvCommandEncoder::encode(
        mpv::MpvCommandRequest{mpv::MpvAudioDelayRequest{
            std::numeric_limits<double>::infinity()}},
        &error);
    QVERIFY(!invalidEncoded.has_value());
    QVERIFY(!error.isEmpty());
}

void AudioDelayFlowTest::propertyObservationOwnsAudioSnapshotWithoutTouchingSubtitle()
{
    mpv::MpvEvent event;
    event.type = mpv::MpvEventType::PropertyChange;
    event.payload = mpv::MpvPropertyChange{
        mpv::MpvPropertyId::AudioDelay,
        mpv::MpvPropertyValue{0.25}};

    auto mapped = mpv::MpvPlaybackEventMapper::map(event);
    QVERIFY(mapped.has_value());
    const auto* delay = std::get_if<domain::AudioDelayChangedEvent>(&mapped->payload);
    QVERIFY(delay != nullptr);
    QVERIFY(delay->seconds.has_value());
    QVERIFY(std::abs(*delay->seconds - 0.25) < 0.0001);

    domain::PlaybackSnapshotState state;
    state.generation = domain::MediaGeneration{71};
    state.lifecycle = domain::PlaybackLifecycleState::Ready;
    state.media.source = QStringLiteral("audio-delay-test.wav");
    state.controls.subtitleDelaySeconds = -0.10;
    mapped->generation = state.generation;

    const domain::PlaybackSnapshot reduced = domain::reducePlaybackSnapshot(
        domain::PlaybackSnapshot{std::move(state)},
        *mapped);
    QVERIFY(reduced.controls().audioDelaySeconds.has_value());
    QVERIFY(std::abs(*reduced.controls().audioDelaySeconds - 0.25) < 0.0001);
    QVERIFY(reduced.controls().subtitleDelaySeconds.has_value());
    QVERIFY(std::abs(*reduced.controls().subtitleDelaySeconds + 0.10) < 0.0001);

    const domain::PlaybackSnapshot subtitleChanged = domain::reducePlaybackSnapshot(
        reduced,
        domain::PlaybackEvent{domain::SubtitleDelayChangedEvent{0.15}});
    QVERIFY(subtitleChanged.controls().audioDelaySeconds.has_value());
    QVERIFY(std::abs(*subtitleChanged.controls().audioDelaySeconds - 0.25) < 0.0001);
    QVERIFY(subtitleChanged.controls().subtitleDelaySeconds.has_value());
    QVERIFY(std::abs(*subtitleChanged.controls().subtitleDelaySeconds - 0.15) < 0.0001);

    const domain::PlaybackSnapshot opening = domain::reducePlaybackSnapshot(
        subtitleChanged,
        domain::PlaybackEvent{domain::MediaLoadStartedEvent{}});
    QVERIFY(!opening.controls().audioDelaySeconds.has_value());
    QVERIFY(!opening.controls().subtitleDelaySeconds.has_value());
}

void AudioDelayFlowTest::sourceChainRefreshesConfirmedAudioDelay()
{
    const QString backend = readSource(
        QStringLiteral("src/playback/application/session/backend/playback_session_backend.cpp"));
    const QString session = readSource(
        QStringLiteral("src/playback/application/session/playback_session.cpp"));
    const QString requestTracker = readSource(
        QStringLiteral("src/playback/application/requests/request_tracker.cpp"));

    QVERIFY(!backend.isEmpty());
    QVERIFY(!session.isEmpty());
    QVERIFY(!requestTracker.isEmpty());

    QVERIFY(backend.contains(QStringLiteral("MpvPropertyId::AudioDelay")));
    QVERIFY(backend.contains(QStringLiteral("refreshAudioDelayState")));
    QVERIFY(session.contains(QStringLiteral("PlaybackRequestType::SetAudioDelay")));
    QVERIFY(session.contains(QStringLiteral("refreshAudioDelayState")));
    QVERIFY(requestTracker.contains(QStringLiteral("PlaybackRequestType::SetAudioDelay")));
    QVERIFY(requestTracker.contains(QStringLiteral("SetAudioDelayCommand")));
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::AudioDelayFlowTest)
#include "audio_delay_flow_test.moc"
