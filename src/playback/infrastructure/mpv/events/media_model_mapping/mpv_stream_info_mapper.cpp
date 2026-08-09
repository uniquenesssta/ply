#include "mpv_stream_info_mapper.h"

#include "mpv_media_model_value_reader.h"
#include "playback/domain/events/stream_event.h"

#include <QMetaType>
#include <QString>
#include <QVariantMap>

#include <utility>

namespace player::playback::mpv {

using namespace player::playback::domain;

std::optional<PlaybackEvent> MpvStreamInfoMapper::mapVideo(
    const MpvPropertyValue& value,
    QString* errorMessage)
{
    bool unavailable = false;
    const QVariant* node = MpvMediaModelValueReader::nodeValue(
        value,
        &unavailable,
        errorMessage);
    if (unavailable) {
        return PlaybackEvent{VideoStreamInfoChangedEvent{std::nullopt}};
    }
    if (node == nullptr) {
        return std::nullopt;
    }
    if (node->metaType().id() != QMetaType::QVariantMap) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("video-params must be an mpv node map."));
        return std::nullopt;
    }

    const QVariantMap map = node->toMap();
    VideoStreamInfo info;
    if (!MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("w"), &info.width, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("h"), &info.height, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("dw"), &info.displayWidth, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("dh"), &info.displayHeight, errorMessage)
        || !MpvMediaModelValueReader::optionalDouble(map, QStringLiteral("aspect"), &info.aspectRatio, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("rotate"), &info.rotationDegrees, errorMessage)
        || !MpvMediaModelValueReader::optionalString(map, QStringLiteral("pixelformat"), &info.pixelFormat, errorMessage)) {
        return std::nullopt;
    }

    return PlaybackEvent{VideoStreamInfoChangedEvent{std::move(info)}};
}

std::optional<PlaybackEvent> MpvStreamInfoMapper::mapAudio(
    const MpvPropertyValue& value,
    QString* errorMessage)
{
    bool unavailable = false;
    const QVariant* node = MpvMediaModelValueReader::nodeValue(
        value,
        &unavailable,
        errorMessage);
    if (unavailable) {
        return PlaybackEvent{AudioStreamInfoChangedEvent{std::nullopt}};
    }
    if (node == nullptr) {
        return std::nullopt;
    }
    if (node->metaType().id() != QMetaType::QVariantMap) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("audio-params must be an mpv node map."));
        return std::nullopt;
    }

    const QVariantMap map = node->toMap();
    AudioStreamInfo info;
    if (!MpvMediaModelValueReader::optionalString(map, QStringLiteral("format"), &info.format, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("samplerate"), &info.sampleRate, errorMessage)
        || !MpvMediaModelValueReader::optionalString(map, QStringLiteral("channels"), &info.channelLayout, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("channel-count"), &info.channelCount, errorMessage)
        || !MpvMediaModelValueReader::optionalString(map, QStringLiteral("hr-channels"), &info.humanReadableChannelLayout, errorMessage)) {
        return std::nullopt;
    }

    return PlaybackEvent{AudioStreamInfoChangedEvent{std::move(info)}};
}

} // namespace player::playback::mpv
