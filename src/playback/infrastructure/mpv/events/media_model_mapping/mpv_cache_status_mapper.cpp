#include "mpv_cache_status_mapper.h"

#include "mpv_media_model_value_reader.h"
#include "playback/domain/events/buffering_event.h"

#include <QMetaType>
#include <QString>
#include <QVariantMap>

#include <utility>

namespace player::playback::mpv {
namespace {

using namespace player::playback::domain;

bool readOptionalCacheBool(
    const QVariantMap& map,
    QStringView key,
    std::optional<bool>* output,
    QString* errorMessage)
{
    const auto it = map.constFind(key.toString());
    if (it == map.cend() || !it->isValid()) {
        output->reset();
        return true;
    }
    if (it->metaType().id() != QMetaType::Bool) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("Cache field '%1' must be boolean when present.").arg(key));
        return false;
    }
    *output = it->toBool();
    return true;
}

} // namespace

std::optional<PlaybackEvent> MpvCacheStatusMapper::map(
    const MpvPropertyValue& value,
    QString* errorMessage)
{
    bool unavailable = false;
    const QVariant* node = MpvMediaModelValueReader::nodeValue(
        value,
        &unavailable,
        errorMessage);
    if (unavailable) {
        return PlaybackEvent{CacheStatusChangedEvent{std::nullopt}};
    }
    if (node == nullptr) {
        return std::nullopt;
    }
    if (node->metaType().id() != QMetaType::QVariantMap) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("demuxer-cache-state must be an mpv node map."));
        return std::nullopt;
    }

    const QVariantMap map = node->toMap();
    CacheStatus status;
    if (!readOptionalCacheBool(map, QStringLiteral("bof-cached"), &status.beginningCached, errorMessage)
        || !readOptionalCacheBool(map, QStringLiteral("eof-cached"), &status.endCached, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("fw-bytes"), &status.forwardBytes, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("total-bytes"), &status.totalBytes, errorMessage)
        || !MpvMediaModelValueReader::optionalDouble(map, QStringLiteral("cache-end"), &status.cacheEndSeconds, errorMessage)
        || !MpvMediaModelValueReader::optionalDouble(map, QStringLiteral("reader-pts"), &status.readerPositionSeconds, errorMessage)
        || !MpvMediaModelValueReader::optionalDouble(map, QStringLiteral("cache-duration"), &status.durationSeconds, errorMessage)
        || !MpvMediaModelValueReader::optionalInteger(map, QStringLiteral("raw-input-rate"), &status.rawInputRateBytesPerSecond, errorMessage)) {
        return std::nullopt;
    }

    return PlaybackEvent{CacheStatusChangedEvent{std::move(status)}};
}

} // namespace player::playback::mpv
