#include "playback/infrastructure/mpv/events/mpv_event_decoder.h"

#include "playback/infrastructure/mpv/errors/mpv_error_mapper.h"
#include "playback/infrastructure/mpv/properties/mpv_node_decoder.h"
#include "playback/infrastructure/mpv/properties/mpv_property_observer.h"

#include <mpv/client.h>

#include <QString>
#include <QVariantMap>

#include <optional>
#include <utility>

namespace player::playback::mpv {
namespace {

MpvEndFileReason mapEndFileReason(mpv_end_file_reason reason) noexcept
{
    switch (reason) {
    case MPV_END_FILE_REASON_EOF:
        return MpvEndFileReason::Eof;
    case MPV_END_FILE_REASON_STOP:
        return MpvEndFileReason::Stop;
    case MPV_END_FILE_REASON_QUIT:
        return MpvEndFileReason::Quit;
    case MPV_END_FILE_REASON_ERROR:
        return MpvEndFileReason::Error;
    case MPV_END_FILE_REASON_REDIRECT:
        return MpvEndFileReason::Redirect;
    default:
        return MpvEndFileReason::Unknown;
    }
}

QString stringOrEmpty(const char* value)
{
    return value == nullptr ? QString{} : QString::fromUtf8(value);
}

std::optional<qint64> commandReplyPlaylistEntryId(const mpv_event& event)
{
    if (event.data == nullptr) {
        return std::nullopt;
    }

    const auto* command = static_cast<const mpv_event_command*>(event.data);
    QString ignoredDiagnostic;
    const auto decodedResult = MpvNodeDecoder::decode(command->result, &ignoredDiagnostic);
    if (!decodedResult.has_value()) {
        return std::nullopt;
    }

    const QVariantMap resultMap = decodedResult->toMap();
    const auto entry = resultMap.constFind(QStringLiteral("playlist_entry_id"));
    if (entry == resultMap.cend()) {
        return std::nullopt;
    }

    bool converted = false;
    const qlonglong playlistEntryId = entry->toLongLong(&converted);
    if (!converted || playlistEntryId <= 0) {
        return std::nullopt;
    }
    return static_cast<qint64>(playlistEntryId);
}

MpvEvent makeBaseEvent(const mpv_event& event, MpvEventType type)
{
    MpvEvent decoded;
    decoded.type = type;
    decoded.replyUserdata = static_cast<quint64>(event.reply_userdata);
    decoded.error = MpvErrorMapper::map(event.error);
    return decoded;
}

MpvEvent makeDecodeFailure(const mpv_event& event, QString diagnostic)
{
    MpvEvent decoded = makeBaseEvent(event, MpvEventType::DecodeFailure);
    decoded.payload = MpvDecodeFailureData{
        static_cast<int>(event.event_id),
        std::move(diagnostic)};
    return decoded;
}

} // namespace

MpvEvent MpvEventDecoder::decode(const mpv_event& event)
{
    switch (event.event_id) {
    case MPV_EVENT_START_FILE: {
        if (event.data == nullptr) {
            return makeDecodeFailure(
                event,
                QStringLiteral("MPV_EVENT_START_FILE did not contain mpv_event_start_file data."));
        }

        const auto* startFile = static_cast<const mpv_event_start_file*>(event.data);
        MpvEvent decoded = makeBaseEvent(event, MpvEventType::StartFile);
        decoded.payload = MpvStartFileData{
            static_cast<qint64>(startFile->playlist_entry_id)};
        return decoded;
    }
    case MPV_EVENT_FILE_LOADED:
        return makeBaseEvent(event, MpvEventType::FileLoaded);
    case MPV_EVENT_END_FILE: {
        if (event.data == nullptr) {
            return makeDecodeFailure(
                event,
                QStringLiteral("MPV_EVENT_END_FILE did not contain mpv_event_end_file data."));
        }

        const auto* endFile = static_cast<const mpv_event_end_file*>(event.data);
        MpvEvent decoded = makeBaseEvent(event, MpvEventType::EndFile);
        decoded.error = MpvErrorMapper::map(endFile->error);
        decoded.payload = MpvEndFileData{
            mapEndFileReason(endFile->reason),
            static_cast<qint64>(endFile->playlist_entry_id),
            static_cast<qint64>(endFile->playlist_insert_id),
            endFile->playlist_insert_num_entries,
            static_cast<int>(endFile->reason)};
        return decoded;
    }
    case MPV_EVENT_COMMAND_REPLY: {
        MpvEvent decoded = makeBaseEvent(event, MpvEventType::CommandReply);
        decoded.payload = MpvCommandReplyData{commandReplyPlaylistEntryId(event)};
        return decoded;
    }
    case MPV_EVENT_PROPERTY_CHANGE: {
        QString propertyError;
        std::optional<MpvPropertyChange> propertyChange;
        if (event.data == nullptr) {
            propertyChange = MpvPropertyObserver::decode(
                static_cast<quint64>(event.reply_userdata),
                MPV_FORMAT_NONE,
                nullptr,
                &propertyError);
        } else {
            const auto* property = static_cast<const mpv_event_property*>(event.data);
            propertyChange = MpvPropertyObserver::decode(
                static_cast<quint64>(event.reply_userdata),
                static_cast<int>(property->format),
                property->data,
                &propertyError);
        }

        if (!propertyChange.has_value()) {
            if (propertyError.isEmpty()) {
                propertyError = QStringLiteral("Unable to decode mpv property-change payload.");
            }
            return makeDecodeFailure(event, propertyError);
        }

        MpvEvent decoded = makeBaseEvent(event, MpvEventType::PropertyChange);
        decoded.payload = *propertyChange;
        return decoded;
    }
    case MPV_EVENT_LOG_MESSAGE: {
        if (event.data == nullptr) {
            return makeDecodeFailure(
                event,
                QStringLiteral("MPV_EVENT_LOG_MESSAGE did not contain mpv_event_log_message data."));
        }

        const auto* logMessage = static_cast<const mpv_event_log_message*>(event.data);
        MpvEvent decoded = makeBaseEvent(event, MpvEventType::LogMessage);
        decoded.payload = MpvLogMessageData{
            stringOrEmpty(logMessage->prefix),
            stringOrEmpty(logMessage->level),
            stringOrEmpty(logMessage->text),
            static_cast<int>(logMessage->log_level)};
        return decoded;
    }
    case MPV_EVENT_SHUTDOWN:
        return makeBaseEvent(event, MpvEventType::Shutdown);
    default: {
        MpvEvent decoded = makeBaseEvent(event, MpvEventType::Unknown);
        decoded.payload = MpvUnknownEventData{static_cast<int>(event.event_id)};
        return decoded;
    }
    }
}

} // namespace player::playback::mpv
