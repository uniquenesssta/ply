#include "playback/infrastructure/mpv/events/mpv_event.h"
#include "playback/infrastructure/mpv/events/mpv_event_decoder.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <mpv/client.h>

#include <QtTest>

#include <variant>

namespace player::playback::mpv {

class MpvEventSemanticsTest final : public QObject
{
    Q_OBJECT

private slots:
    void endFilePreservesReasonErrorAndBoundaryMetadata();
    void propertyNullAndNoneRemainTypedUnavailable();
    void commandReplyPreservesRequestIdentityAndError();
    void unknownMissingAndNullStringsAreSafe();
};

void MpvEventSemanticsTest::endFilePreservesReasonErrorAndBoundaryMetadata()
{
    struct ReasonCase final
    {
        mpv_end_file_reason raw;
        MpvEndFileReason typed;
    };

    const ReasonCase cases[]{
        {MPV_END_FILE_REASON_EOF, MpvEndFileReason::Eof},
        {MPV_END_FILE_REASON_STOP, MpvEndFileReason::Stop},
        {MPV_END_FILE_REASON_QUIT, MpvEndFileReason::Quit},
        {MPV_END_FILE_REASON_ERROR, MpvEndFileReason::Error},
        {MPV_END_FILE_REASON_REDIRECT, MpvEndFileReason::Redirect},
    };

    for (const ReasonCase& reasonCase : cases) {
        mpv_event_end_file endData{};
        endData.reason = reasonCase.raw;
        endData.error = reasonCase.raw == MPV_END_FILE_REASON_ERROR
                            ? MPV_ERROR_LOADING_FAILED
                            : MPV_ERROR_SUCCESS;
        endData.playlist_entry_id = 41;
        endData.playlist_insert_id = 42;
        endData.playlist_insert_num_entries = 3;

        mpv_event raw{};
        raw.event_id = MPV_EVENT_END_FILE;
        raw.data = &endData;

        const MpvEvent decoded = MpvEventDecoder::decode(raw);
        QCOMPARE(decoded.type, MpvEventType::EndFile);
        const MpvEndFileData payload = std::get<MpvEndFileData>(decoded.payload);
        QCOMPARE(payload.reason, reasonCase.typed);
        QCOMPARE(payload.rawReason, static_cast<int>(reasonCase.raw));
        QCOMPARE(payload.playlistEntryId, qint64{41});
        QCOMPARE(payload.playlistInsertId, qint64{42});
        QCOMPARE(payload.playlistInsertNumEntries, 3);

        if (reasonCase.raw == MPV_END_FILE_REASON_ERROR) {
            QCOMPARE(decoded.error.code, MpvErrorCode::LoadingFailed);
        } else {
            QVERIFY(decoded.error.isSuccess());
        }
    }

    constexpr int kUnknownReason = 9001;
    mpv_event_end_file unknownData{};
    unknownData.reason = static_cast<mpv_end_file_reason>(kUnknownReason);

    mpv_event unknownRaw{};
    unknownRaw.event_id = MPV_EVENT_END_FILE;
    unknownRaw.data = &unknownData;

    const MpvEvent unknownDecoded = MpvEventDecoder::decode(unknownRaw);
    const MpvEndFileData unknownPayload = std::get<MpvEndFileData>(unknownDecoded.payload);
    QCOMPARE(unknownPayload.reason, MpvEndFileReason::Unknown);
    QCOMPARE(unknownPayload.rawReason, kUnknownReason);
}

void MpvEventSemanticsTest::propertyNullAndNoneRemainTypedUnavailable()
{
    const MpvPropertyDefinition* pause = MpvPropertyRegistry::findById(MpvPropertyId::Pause);
    QVERIFY(pause != nullptr);

    mpv_event noPayload{};
    noPayload.event_id = MPV_EVENT_PROPERTY_CHANGE;
    noPayload.reply_userdata = pause->observationId;

    const MpvEvent decodedNoPayload = MpvEventDecoder::decode(noPayload);
    QCOMPARE(decodedNoPayload.type, MpvEventType::PropertyChange);
    const MpvPropertyChange noPayloadChange =
        std::get<MpvPropertyChange>(decodedNoPayload.payload);
    QCOMPARE(noPayloadChange.id, MpvPropertyId::Pause);
    QVERIFY(std::holds_alternative<std::monostate>(noPayloadChange.value));

    mpv_event_property noneProperty{};
    noneProperty.name = pause->name.constData();
    noneProperty.format = MPV_FORMAT_NONE;
    noneProperty.data = nullptr;

    mpv_event noneRaw{};
    noneRaw.event_id = MPV_EVENT_PROPERTY_CHANGE;
    noneRaw.reply_userdata = pause->observationId;
    noneRaw.data = &noneProperty;

    const MpvEvent decodedNone = MpvEventDecoder::decode(noneRaw);
    QCOMPARE(decodedNone.type, MpvEventType::PropertyChange);
    const MpvPropertyChange noneChange = std::get<MpvPropertyChange>(decodedNone.payload);
    QVERIFY(std::holds_alternative<std::monostate>(noneChange.value));

    mpv_event_property nullDataProperty{};
    nullDataProperty.name = pause->name.constData();
    nullDataProperty.format = MPV_FORMAT_FLAG;
    nullDataProperty.data = nullptr;

    mpv_event nullDataRaw{};
    nullDataRaw.event_id = MPV_EVENT_PROPERTY_CHANGE;
    nullDataRaw.reply_userdata = pause->observationId;
    nullDataRaw.data = &nullDataProperty;

    const MpvEvent decodedNullData = MpvEventDecoder::decode(nullDataRaw);
    QCOMPARE(decodedNullData.type, MpvEventType::PropertyChange);
    const MpvPropertyChange nullDataChange =
        std::get<MpvPropertyChange>(decodedNullData.payload);
    QVERIFY(std::holds_alternative<std::monostate>(nullDataChange.value));
}

void MpvEventSemanticsTest::commandReplyPreservesRequestIdentityAndError()
{
    mpv_event successRaw{};
    successRaw.event_id = MPV_EVENT_COMMAND_REPLY;
    successRaw.reply_userdata = 501;
    successRaw.error = MPV_ERROR_SUCCESS;

    const MpvEvent success = MpvEventDecoder::decode(successRaw);
    QCOMPARE(success.type, MpvEventType::CommandReply);
    QCOMPARE(success.replyUserdata, quint64{501});
    QVERIFY(success.error.isSuccess());

    mpv_event errorRaw{};
    errorRaw.event_id = MPV_EVENT_COMMAND_REPLY;
    errorRaw.reply_userdata = 502;
    errorRaw.error = MPV_ERROR_COMMAND;

    const MpvEvent failure = MpvEventDecoder::decode(errorRaw);
    QCOMPARE(failure.type, MpvEventType::CommandReply);
    QCOMPARE(failure.replyUserdata, quint64{502});
    QCOMPARE(failure.error.code, MpvErrorCode::Command);
    QCOMPARE(failure.error.rawCode, MPV_ERROR_COMMAND);
}

void MpvEventSemanticsTest::unknownMissingAndNullStringsAreSafe()
{
    mpv_event unknown{};
    unknown.event_id = static_cast<mpv_event_id>(9999);

    const MpvEvent unknownDecoded = MpvEventDecoder::decode(unknown);
    QCOMPARE(unknownDecoded.type, MpvEventType::Unknown);
    QCOMPARE(std::get<MpvUnknownEventData>(unknownDecoded.payload).rawEventId, 9999);

    mpv_event missingStart{};
    missingStart.event_id = MPV_EVENT_START_FILE;
    QCOMPARE(MpvEventDecoder::decode(missingStart).type, MpvEventType::DecodeFailure);

    mpv_event missingEnd{};
    missingEnd.event_id = MPV_EVENT_END_FILE;
    QCOMPARE(MpvEventDecoder::decode(missingEnd).type, MpvEventType::DecodeFailure);

    mpv_event_log_message nullStrings{};
    nullStrings.prefix = nullptr;
    nullStrings.level = nullptr;
    nullStrings.text = nullptr;
    nullStrings.log_level = MPV_LOG_LEVEL_INFO;

    mpv_event logRaw{};
    logRaw.event_id = MPV_EVENT_LOG_MESSAGE;
    logRaw.data = &nullStrings;

    const MpvEvent decodedLog = MpvEventDecoder::decode(logRaw);
    QCOMPARE(decodedLog.type, MpvEventType::LogMessage);
    const MpvLogMessageData log = std::get<MpvLogMessageData>(decodedLog.payload);
    QVERIFY(log.prefix.isEmpty());
    QVERIFY(log.level.isEmpty());
    QVERIFY(log.text.isEmpty());
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvEventSemanticsTest)
#include "mpv_event_semantics_test.moc"
