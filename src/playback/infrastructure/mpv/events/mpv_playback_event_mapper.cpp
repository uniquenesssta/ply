#include "mpv_playback_event_mapper.h"

#include "mpv_media_model_mapper.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <QVariant>

#include <cmath>
#include <utility>
#include <variant>

namespace player::playback::mpv {
namespace {

using namespace player::playback::domain;

template <typename T>
struct ScalarReadResult final
{
    bool validType = false;
    std::optional<T> value;
};

template <typename T>
ScalarReadResult<T> readScalar(const MpvPropertyValue& value)
{
    if (std::holds_alternative<std::monostate>(value)) {
        return ScalarReadResult<T>{true, std::nullopt};
    }
    if (const T* typedValue = std::get_if<T>(&value)) {
        return ScalarReadResult<T>{true, *typedValue};
    }
    return ScalarReadResult<T>{false, std::nullopt};
}

PlaybackFailure makeFailure(
    PlaybackFailureCategory category,
    int backendCode,
    QString diagnostic)
{
    return PlaybackFailure{category, backendCode, std::move(diagnostic)};
}

PlaybackEvent makeProtocolFailure(QString diagnostic)
{
    if (diagnostic.isEmpty()) {
        diagnostic = QStringLiteral("Playback backend property could not be mapped into a domain value.");
    }
    return PlaybackEvent{PlaybackFailureEvent{
        makeFailure(PlaybackFailureCategory::Protocol, 0, std::move(diagnostic))}};
}

PlaybackFailure mapBackendFailure(
    const MpvError& error,
    PlaybackFailureCategory category)
{
    return makeFailure(
        category,
        error.rawCode,
        error.message.isEmpty() ? QStringLiteral("Playback backend operation failed.") : error.message);
}

MediaEndReason mapEndReason(MpvEndFileReason reason)
{
    switch (reason) {
    case MpvEndFileReason::Eof:
        return MediaEndReason::Eof;
    case MpvEndFileReason::Stop:
        return MediaEndReason::Stopped;
    case MpvEndFileReason::Quit:
        return MediaEndReason::Shutdown;
    case MpvEndFileReason::Redirect:
        return MediaEndReason::Redirected;
    case MpvEndFileReason::Unknown:
    case MpvEndFileReason::Error:
        return MediaEndReason::Unknown;
    }

    return MediaEndReason::Unknown;
}

std::optional<PlaybackEvent> mapComplexMediaProperty(const MpvPropertyChange& change)
{
    QString diagnostic;
    auto mapped = MpvMediaModelMapper::map(change, &diagnostic);
    if (mapped.has_value()) {
        return mapped;
    }
    return makeProtocolFailure(std::move(diagnostic));
}

std::optional<PlaybackEvent> mapProperty(const MpvPropertyChange& change)
{
    const auto invalidType = [&change]() -> std::optional<PlaybackEvent> {
        return makeProtocolFailure(
            QStringLiteral("Unexpected typed payload for playback property id %1.")
                .arg(static_cast<quint16>(change.id)));
    };

    switch (change.id) {
    case MpvPropertyId::Position: {
        const auto value = readScalar<double>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{PositionChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::Duration: {
        const auto value = readScalar<double>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{DurationChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::Pause: {
        const auto value = readScalar<bool>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{PauseChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::Volume: {
        const auto value = readScalar<double>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{VolumeChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::Mute: {
        const auto value = readScalar<bool>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{MuteChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::Speed: {
        const auto value = readScalar<double>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{SpeedChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::SubtitleDelay: {
        const auto value = readScalar<double>(change.value);
        if (!value.validType) {
            return invalidType();
        }
        if (value.value.has_value() && !std::isfinite(*value.value)) {
            return invalidType();
        }
        return PlaybackEvent{SubtitleDelayChangedEvent{value.value}};
    }
    case MpvPropertyId::AudioDelay: {
        const auto value = readScalar<double>(change.value);
        if (!value.validType) {
            return invalidType();
        }
        if (value.value.has_value() && !std::isfinite(*value.value)) {
            return invalidType();
        }
        return PlaybackEvent{AudioDelayChangedEvent{value.value}};
    }
    case MpvPropertyId::Seekable: {
        const auto value = readScalar<bool>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{SeekableChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::CoreIdle: {
        const auto value = readScalar<bool>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{CoreIdleChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::EofReached: {
        const auto value = readScalar<bool>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{EofReachedChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::Seeking: {
        const auto value = readScalar<bool>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{SeekingChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::PausedForCache: {
        const auto value = readScalar<bool>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{BufferingChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::CacheBufferingState: {
        if (std::holds_alternative<std::monostate>(change.value)) {
            return PlaybackEvent{BufferingProgressChangedEvent{std::nullopt}};
        }
        const QVariant* node = std::get_if<QVariant>(&change.value);
        if (node == nullptr || !node->isValid()) {
            return node == nullptr ? invalidType()
                                   : std::optional<PlaybackEvent>{PlaybackEvent{
                                         BufferingProgressChangedEvent{std::nullopt}}};
        }
        bool converted = false;
        const double percent = node->toDouble(&converted);
        if (!converted || !std::isfinite(percent)) {
            return invalidType();
        }
        return PlaybackEvent{BufferingProgressChangedEvent{percent}};
    }
    case MpvPropertyId::MediaTitle: {
        const auto value = readScalar<QString>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{MediaTitleChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::Path: {
        const auto value = readScalar<QString>(change.value);
        return value.validType ? std::optional<PlaybackEvent>{PlaybackEvent{MediaPathChangedEvent{value.value}}}
                               : invalidType();
    }
    case MpvPropertyId::TrackList:
    case MpvPropertyId::ChapterList:
    case MpvPropertyId::DemuxerCacheState:
    case MpvPropertyId::SelectedAudioTrack:
    case MpvPropertyId::SelectedSubtitleTrack:
    case MpvPropertyId::SelectedVideoTrack:
    case MpvPropertyId::VideoParams:
    case MpvPropertyId::AudioParams:
        return mapComplexMediaProperty(change);
    }

    return std::nullopt;
}

} // namespace

std::optional<PlaybackEvent> MpvPlaybackEventMapper::map(const MpvEvent& event)
{
    switch (event.type) {
    case MpvEventType::StartFile:
        if (!std::holds_alternative<MpvStartFileData>(event.payload)) {
            return makeProtocolFailure(QStringLiteral("start-file event is missing typed payload data."));
        }
        return PlaybackEvent{MediaLoadStartedEvent{}};

    case MpvEventType::FileLoaded:
        return PlaybackEvent{MediaLoadedEvent{}};

    case MpvEventType::EndFile: {
        const auto* endFile = std::get_if<MpvEndFileData>(&event.payload);
        if (endFile == nullptr) {
            return makeProtocolFailure(QStringLiteral("end-file event is missing typed payload data."));
        }
        if (endFile->reason == MpvEndFileReason::Error) {
            return PlaybackEvent{MediaFailedEvent{
                mapBackendFailure(event.error, PlaybackFailureCategory::Media)}};
        }
        return PlaybackEvent{MediaEndedEvent{mapEndReason(endFile->reason)}};
    }

    case MpvEventType::CommandReply: {
        const player::ids::RequestId requestId{event.replyUserdata};
        if (!requestId.isValid()) {
            return makeProtocolFailure(QStringLiteral("command reply is missing a valid request id."));
        }
        if (event.error.isSuccess()) {
            return PlaybackEvent{CommandReplyEvent{requestId, true, std::nullopt}};
        }
        return PlaybackEvent{CommandReplyEvent{
            requestId,
            false,
            mapBackendFailure(event.error, PlaybackFailureCategory::Command)}};
    }

    case MpvEventType::PropertyChange: {
        const auto* change = std::get_if<MpvPropertyChange>(&event.payload);
        if (change == nullptr) {
            return makeProtocolFailure(QStringLiteral("property-change event is missing typed payload data."));
        }
        return mapProperty(*change);
    }

    case MpvEventType::Shutdown:
        return PlaybackEvent{PlaybackBackendShutdownEvent{}};

    case MpvEventType::DecodeFailure: {
        const auto* failure = std::get_if<MpvDecodeFailureData>(&event.payload);
        return PlaybackEvent{PlaybackFailureEvent{makeFailure(
            PlaybackFailureCategory::Protocol,
            0,
            failure == nullptr || failure->diagnostic.isEmpty()
                ? QStringLiteral("Playback backend event decoding failed.")
                : failure->diagnostic)}};
    }

    case MpvEventType::LogMessage:
    case MpvEventType::Unknown:
        return std::nullopt;
    }

    return std::nullopt;
}

} // namespace player::playback::mpv
