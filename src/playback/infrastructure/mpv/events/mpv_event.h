#pragma once

#include "playback/infrastructure/mpv/errors/mpv_error_mapper.h"
#include "playback/infrastructure/mpv/properties/mpv_property_change.h"

#include <QMetaType>
#include <QString>
#include <QtGlobal>

#include <variant>

namespace player::playback::mpv {

enum class MpvEventType
{
    StartFile,
    FileLoaded,
    EndFile,
    CommandReply,
    PropertyChange,
    LogMessage,
    Shutdown,
    Unknown,
    DecodeFailure,
};

enum class MpvEndFileReason
{
    Eof,
    Stop,
    Quit,
    Error,
    Redirect,
    Unknown,
};

struct MpvStartFileData final
{
    qint64 playlistEntryId = 0;
};

struct MpvEndFileData final
{
    MpvEndFileReason reason = MpvEndFileReason::Unknown;
    qint64 playlistEntryId = 0;
    qint64 playlistInsertId = 0;
    int playlistInsertNumEntries = 0;
};

struct MpvLogMessageData final
{
    QString prefix;
    QString level;
    QString text;
    int numericLevel = 0;
};

struct MpvUnknownEventData final
{
    int rawEventId = 0;
};

struct MpvDecodeFailureData final
{
    int rawEventId = 0;
    QString diagnostic;
};

using MpvEventPayload = std::variant<
    std::monostate,
    MpvStartFileData,
    MpvEndFileData,
    MpvPropertyChange,
    MpvLogMessageData,
    MpvUnknownEventData,
    MpvDecodeFailureData>;

struct MpvEvent final
{
    MpvEventType type = MpvEventType::Unknown;
    quint64 replyUserdata = 0;
    MpvError error;
    MpvEventPayload payload;
};

} // namespace player::playback::mpv

Q_DECLARE_METATYPE(player::playback::mpv::MpvEvent)
