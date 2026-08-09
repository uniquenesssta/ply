#include "playback/domain/events/playback_event.h"
#include "playback/infrastructure/mpv/errors/mpv_error_mapper.h"
#include "playback/infrastructure/mpv/events/mpv_playback_event_mapper.h"
#include "playback/infrastructure/mpv/properties/mpv_property_change.h"

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QtTest/QTest>

#include <variant>

namespace player::playback::mpv {

using namespace player::playback::domain;

class PlaybackEventMappingTest final : public QObject
{
    Q_OBJECT

private slots:
    void mapsMediaLifecycle();
    void mapsEndReasonsAndFailures();
    void mapsCommandReplies();
    void mapsCoreProperties();
    void mapsTypedMediaStateProperties();
    void mapsUnavailableProperty();
    void rejectsUnexpectedPropertyShape();
    void ignoresNonDomainEvents();
};

void PlaybackEventMappingTest::mapsMediaLifecycle()
{
    MpvEvent start;
    start.type = MpvEventType::StartFile;
    start.payload = MpvStartFileData{42};
    const auto mappedStart = MpvPlaybackEventMapper::map(start);
    QVERIFY(mappedStart.has_value());
    QVERIFY(std::holds_alternative<MediaLoadStartedEvent>(mappedStart->payload));

    MpvEvent loaded;
    loaded.type = MpvEventType::FileLoaded;
    const auto mappedLoaded = MpvPlaybackEventMapper::map(loaded);
    QVERIFY(mappedLoaded.has_value());
    QVERIFY(std::holds_alternative<MediaLoadedEvent>(mappedLoaded->payload));

    MpvEvent shutdown;
    shutdown.type = MpvEventType::Shutdown;
    const auto mappedShutdown = MpvPlaybackEventMapper::map(shutdown);
    QVERIFY(mappedShutdown.has_value());
    QVERIFY(std::holds_alternative<PlaybackBackendShutdownEvent>(mappedShutdown->payload));
}

void PlaybackEventMappingTest::mapsEndReasonsAndFailures()
{
    MpvEvent eof;
    eof.type = MpvEventType::EndFile;
    eof.payload = MpvEndFileData{MpvEndFileReason::Eof};
    const auto mappedEof = MpvPlaybackEventMapper::map(eof);
    QVERIFY(mappedEof.has_value());
    const auto* eofEvent = std::get_if<MediaEndedEvent>(&mappedEof->payload);
    QVERIFY(eofEvent != nullptr);
    QCOMPARE(eofEvent->reason, MediaEndReason::Eof);

    MpvEvent failed;
    failed.type = MpvEventType::EndFile;
    failed.error = MpvError{MpvErrorCode::LoadingFailed, -13, QStringLiteral("loading failed")};
    failed.payload = MpvEndFileData{MpvEndFileReason::Error};
    const auto mappedFailed = MpvPlaybackEventMapper::map(failed);
    QVERIFY(mappedFailed.has_value());
    const auto* failureEvent = std::get_if<MediaFailedEvent>(&mappedFailed->payload);
    QVERIFY(failureEvent != nullptr);
    QCOMPARE(failureEvent->failure.category, PlaybackFailureCategory::Media);
    QCOMPARE(failureEvent->failure.backendCode, -13);
    QCOMPARE(failureEvent->failure.diagnostic, QStringLiteral("loading failed"));
}

void PlaybackEventMappingTest::mapsCommandReplies()
{
    MpvEvent success;
    success.type = MpvEventType::CommandReply;
    success.replyUserdata = 91;
    const auto mappedSuccess = MpvPlaybackEventMapper::map(success);
    QVERIFY(mappedSuccess.has_value());
    const auto* successEvent = std::get_if<CommandReplyEvent>(&mappedSuccess->payload);
    QVERIFY(successEvent != nullptr);
    QCOMPARE(successEvent->requestId.value(), quint64{91});
    QVERIFY(successEvent->succeeded);
    QVERIFY(!successEvent->failure.has_value());

    MpvEvent failed;
    failed.type = MpvEventType::CommandReply;
    failed.replyUserdata = 92;
    failed.error = MpvError{MpvErrorCode::Command, -12, QStringLiteral("command failed")};
    const auto mappedFailed = MpvPlaybackEventMapper::map(failed);
    QVERIFY(mappedFailed.has_value());
    const auto* failedEvent = std::get_if<CommandReplyEvent>(&mappedFailed->payload);
    QVERIFY(failedEvent != nullptr);
    QVERIFY(!failedEvent->succeeded);
    QVERIFY(failedEvent->failure.has_value());
    QCOMPARE(failedEvent->failure->category, PlaybackFailureCategory::Command);
}

void PlaybackEventMappingTest::mapsCoreProperties()
{
    MpvEvent position;
    position.type = MpvEventType::PropertyChange;
    position.payload = MpvPropertyChange{MpvPropertyId::Position, 12.5};
    const auto mappedPosition = MpvPlaybackEventMapper::map(position);
    QVERIFY(mappedPosition.has_value());
    const auto* positionEvent = std::get_if<PositionChangedEvent>(&mappedPosition->payload);
    QVERIFY(positionEvent != nullptr);
    QVERIFY(positionEvent->seconds.has_value());
    QCOMPARE(*positionEvent->seconds, 12.5);

    MpvEvent buffering;
    buffering.type = MpvEventType::PropertyChange;
    buffering.payload = MpvPropertyChange{
        MpvPropertyId::PausedForCache,
        true};
    const auto mappedBuffering = MpvPlaybackEventMapper::map(buffering);
    QVERIFY(mappedBuffering.has_value());
    const auto* bufferingEvent = std::get_if<BufferingChangedEvent>(&mappedBuffering->payload);
    QVERIFY(bufferingEvent != nullptr);
    QVERIFY(bufferingEvent->buffering.has_value());
    QVERIFY(*bufferingEvent->buffering);

    MpvEvent bufferingProgress;
    bufferingProgress.type = MpvEventType::PropertyChange;
    bufferingProgress.payload = MpvPropertyChange{
        MpvPropertyId::CacheBufferingState,
        QVariant::fromValue<qint64>(64)};
    const auto mappedProgress = MpvPlaybackEventMapper::map(bufferingProgress);
    QVERIFY(mappedProgress.has_value());
    const auto* progressEvent = std::get_if<BufferingProgressChangedEvent>(&mappedProgress->payload);
    QVERIFY(progressEvent != nullptr);
    QVERIFY(progressEvent->percent.has_value());
    QCOMPARE(*progressEvent->percent, 64.0);

    MpvEvent title;
    title.type = MpvEventType::PropertyChange;
    title.payload = MpvPropertyChange{
        MpvPropertyId::MediaTitle,
        QStringLiteral("Domain title")};
    const auto mappedTitle = MpvPlaybackEventMapper::map(title);
    QVERIFY(mappedTitle.has_value());
    const auto* titleEvent = std::get_if<MediaTitleChangedEvent>(&mappedTitle->payload);
    QVERIFY(titleEvent != nullptr);
    QVERIFY(titleEvent->title.has_value());
    QCOMPARE(*titleEvent->title, QStringLiteral("Domain title"));
}

void PlaybackEventMappingTest::mapsTypedMediaStateProperties()
{
    QVariantMap audioTrack;
    audioTrack.insert(QStringLiteral("id"), QVariant::fromValue<qlonglong>(3));
    audioTrack.insert(QStringLiteral("type"), QStringLiteral("audio"));
    audioTrack.insert(QStringLiteral("selected"), true);

    MpvEvent trackList;
    trackList.type = MpvEventType::PropertyChange;
    trackList.payload = MpvPropertyChange{
        MpvPropertyId::TrackList,
        QVariant{QVariantList{audioTrack}}};
    const auto mappedTracks = MpvPlaybackEventMapper::map(trackList);
    QVERIFY(mappedTracks.has_value());
    const auto* tracks = std::get_if<TrackListChangedEvent>(&mappedTracks->payload);
    QVERIFY(tracks != nullptr);
    QCOMPARE(tracks->tracks.size(), qsizetype{1});
    QCOMPARE(tracks->tracks.front().kind, TrackKind::Audio);

    MpvEvent selectedAudio;
    selectedAudio.type = MpvEventType::PropertyChange;
    selectedAudio.payload = MpvPropertyChange{
        MpvPropertyId::SelectedAudioTrack,
        QVariant::fromValue<qlonglong>(3)};
    const auto mappedSelection = MpvPlaybackEventMapper::map(selectedAudio);
    QVERIFY(mappedSelection.has_value());
    const auto* selection = std::get_if<SelectedAudioTrackChangedEvent>(&mappedSelection->payload);
    QVERIFY(selection != nullptr);
    QVERIFY(selection->trackId.has_value());
    QCOMPARE(*selection->trackId, qint64{3});

    QVariantMap audioParams;
    audioParams.insert(QStringLiteral("samplerate"), QVariant::fromValue<qlonglong>(48000));
    MpvEvent audioInfo;
    audioInfo.type = MpvEventType::PropertyChange;
    audioInfo.payload = MpvPropertyChange{
        MpvPropertyId::AudioParams,
        QVariant{audioParams}};
    const auto mappedAudio = MpvPlaybackEventMapper::map(audioInfo);
    QVERIFY(mappedAudio.has_value());
    QVERIFY(std::holds_alternative<AudioStreamInfoChangedEvent>(mappedAudio->payload));
}

void PlaybackEventMappingTest::mapsUnavailableProperty()
{
    MpvEvent duration;
    duration.type = MpvEventType::PropertyChange;
    duration.payload = MpvPropertyChange{MpvPropertyId::Duration, std::monostate{}};

    const auto mapped = MpvPlaybackEventMapper::map(duration);
    QVERIFY(mapped.has_value());
    const auto* durationEvent = std::get_if<DurationChangedEvent>(&mapped->payload);
    QVERIFY(durationEvent != nullptr);
    QVERIFY(!durationEvent->seconds.has_value());
}

void PlaybackEventMappingTest::rejectsUnexpectedPropertyShape()
{
    MpvEvent malformed;
    malformed.type = MpvEventType::PropertyChange;
    malformed.payload = MpvPropertyChange{
        MpvPropertyId::Pause,
        QStringLiteral("yes")};

    const auto mapped = MpvPlaybackEventMapper::map(malformed);
    QVERIFY(mapped.has_value());
    const auto* failure = std::get_if<PlaybackFailureEvent>(&mapped->payload);
    QVERIFY(failure != nullptr);
    QCOMPARE(failure->failure.category, PlaybackFailureCategory::Protocol);

    QVariantMap malformedTrack;
    malformedTrack.insert(QStringLiteral("type"), QStringLiteral("audio"));
    MpvEvent malformedTrackList;
    malformedTrackList.type = MpvEventType::PropertyChange;
    malformedTrackList.payload = MpvPropertyChange{
        MpvPropertyId::TrackList,
        QVariant{QVariantList{malformedTrack}}};
    const auto mappedMalformedTrack = MpvPlaybackEventMapper::map(malformedTrackList);
    QVERIFY(mappedMalformedTrack.has_value());
    const auto* trackFailure = std::get_if<PlaybackFailureEvent>(&mappedMalformedTrack->payload);
    QVERIFY(trackFailure != nullptr);
    QCOMPARE(trackFailure->failure.category, PlaybackFailureCategory::Protocol);
}

void PlaybackEventMappingTest::ignoresNonDomainEvents()
{
    MpvEvent log;
    log.type = MpvEventType::LogMessage;
    log.payload = MpvLogMessageData{};
    QVERIFY(!MpvPlaybackEventMapper::map(log).has_value());

    MpvEvent unknown;
    unknown.type = MpvEventType::Unknown;
    unknown.payload = MpvUnknownEventData{999};
    QVERIFY(!MpvPlaybackEventMapper::map(unknown).has_value());
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::PlaybackEventMappingTest)
#include "playback_event_mapping_test.moc"
