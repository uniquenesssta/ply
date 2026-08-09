#include "mpv_track_model_mapper.h"

#include "mpv_media_model_value_reader.h"
#include "playback/domain/events/track_event.h"

#include <QMetaType>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <optional>
#include <utility>

namespace player::playback::mpv {
namespace {

using namespace player::playback::domain;

bool readRequiredPositiveId(
    const QVariantMap& map,
    qint64* output,
    QString* errorMessage)
{
    const auto it = map.constFind(QStringLiteral("id"));
    if (it == map.cend() || !it->isValid() || !MpvMediaModelValueReader::isInteger(*it)) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("Track entry is missing a numeric 'id' field."));
        return false;
    }

    bool converted = false;
    const qint64 value = it->toLongLong(&converted);
    if (!converted || value <= 0) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("Track entry contains an invalid 'id' value."));
        return false;
    }
    *output = value;
    return true;
}

bool readRequiredType(
    const QVariantMap& map,
    QString* output,
    QString* errorMessage)
{
    const auto it = map.constFind(QStringLiteral("type"));
    if (it == map.cend() || !it->isValid() || it->metaType().id() != QMetaType::QString) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("Track entry is missing a string 'type' field."));
        return false;
    }
    *output = it->toString();
    return true;
}

std::optional<TrackKind> kindFromString(const QString& type)
{
    if (type == QStringLiteral("video")) {
        return TrackKind::Video;
    }
    if (type == QStringLiteral("audio")) {
        return TrackKind::Audio;
    }
    if (type == QStringLiteral("sub")) {
        return TrackKind::Subtitle;
    }
    return std::nullopt;
}

bool readSelectionId(
    const MpvPropertyValue& value,
    std::optional<qint64>* trackId,
    QString* errorMessage)
{
    bool unavailable = false;
    const QVariant* node = MpvMediaModelValueReader::nodeValue(
        value,
        &unavailable,
        errorMessage);
    if (unavailable) {
        trackId->reset();
        return true;
    }
    if (node == nullptr) {
        return false;
    }

    if (node->metaType().id() == QMetaType::QString) {
        const QString text = node->toString().trimmed();
        if (text.isEmpty() || text == QStringLiteral("no") || text == QStringLiteral("auto")) {
            trackId->reset();
            return true;
        }
        bool converted = false;
        const qint64 id = text.toLongLong(&converted);
        if (!converted || id <= 0) {
            MpvMediaModelValueReader::setError(
                errorMessage,
                QStringLiteral("Selected track property contains an unsupported string value '%1'.")
                    .arg(text));
            return false;
        }
        *trackId = id;
        return true;
    }

    if (!MpvMediaModelValueReader::isInteger(*node)) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("Selected track property must be an integer id, 'no', or 'auto'."));
        return false;
    }

    bool converted = false;
    const qint64 id = node->toLongLong(&converted);
    if (!converted || id <= 0) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("Selected track property contains an invalid id."));
        return false;
    }
    *trackId = id;
    return true;
}

} // namespace

std::optional<PlaybackEvent> MpvTrackModelMapper::mapTrackList(
    const MpvPropertyValue& value,
    QString* errorMessage)
{
    bool unavailable = false;
    const QVariant* node = MpvMediaModelValueReader::nodeValue(
        value,
        &unavailable,
        errorMessage);
    if (unavailable) {
        return PlaybackEvent{TrackListChangedEvent{}};
    }
    if (node == nullptr) {
        return std::nullopt;
    }
    if (node->metaType().id() != QMetaType::QVariantList) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("track-list must be an mpv node array."));
        return std::nullopt;
    }

    QList<TrackDescriptor> tracks;
    const QVariantList list = node->toList();
    tracks.reserve(list.size());
    for (const QVariant& item : list) {
        if (item.metaType().id() != QMetaType::QVariantMap) {
            MpvMediaModelValueReader::setError(
                errorMessage,
                QStringLiteral("track-list contains a non-map entry."));
            return std::nullopt;
        }

        const QVariantMap map = item.toMap();
        TrackDescriptor track;
        QString type;
        if (!readRequiredPositiveId(map, &track.id, errorMessage)
            || !readRequiredType(map, &type, errorMessage)) {
            return std::nullopt;
        }

        const auto kind = kindFromString(type);
        if (!kind.has_value()) {
            MpvMediaModelValueReader::setError(
                errorMessage,
                QStringLiteral("track-list contains an unsupported track type '%1'.").arg(type));
            return std::nullopt;
        }
        track.kind = *kind;

        if (!MpvMediaModelValueReader::optionalString(map, QStringLiteral("title"), &track.title, errorMessage)
            || !MpvMediaModelValueReader::optionalString(map, QStringLiteral("lang"), &track.language, errorMessage)
            || !MpvMediaModelValueReader::optionalString(map, QStringLiteral("codec"), &track.codec, errorMessage)
            || !MpvMediaModelValueReader::optionalString(map, QStringLiteral("external-filename"), &track.externalFilename, errorMessage)
            || !MpvMediaModelValueReader::optionalBool(map, QStringLiteral("selected"), &track.selected, errorMessage)
            || !MpvMediaModelValueReader::optionalBool(map, QStringLiteral("default"), &track.defaultTrack, errorMessage)
            || !MpvMediaModelValueReader::optionalBool(map, QStringLiteral("forced"), &track.forced, errorMessage)
            || !MpvMediaModelValueReader::optionalBool(map, QStringLiteral("external"), &track.external, errorMessage)
            || !MpvMediaModelValueReader::optionalBool(map, QStringLiteral("image"), &track.image, errorMessage)
            || !MpvMediaModelValueReader::optionalBool(map, QStringLiteral("albumart"), &track.albumArt, errorMessage)) {
            return std::nullopt;
        }

        tracks.append(std::move(track));
    }

    return PlaybackEvent{TrackListChangedEvent{std::move(tracks)}};
}

std::optional<PlaybackEvent> MpvTrackModelMapper::mapSelection(
    MpvPropertyId id,
    const MpvPropertyValue& value,
    QString* errorMessage)
{
    std::optional<qint64> trackId;
    if (!readSelectionId(value, &trackId, errorMessage)) {
        return std::nullopt;
    }

    switch (id) {
    case MpvPropertyId::SelectedVideoTrack:
        return PlaybackEvent{SelectedVideoTrackChangedEvent{trackId}};
    case MpvPropertyId::SelectedAudioTrack:
        return PlaybackEvent{SelectedAudioTrackChangedEvent{trackId}};
    case MpvPropertyId::SelectedSubtitleTrack:
        return PlaybackEvent{SelectedSubtitleTrackChangedEvent{trackId}};
    default:
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("Unsupported selected-track property id."));
        return std::nullopt;
    }
}

} // namespace player::playback::mpv
