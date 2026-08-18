#include "mpv_track_list_decoder.h"

#include "mpv_media_model_value_reader.h"

#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <optional>
#include <utility>

namespace player::playback::mpv {
namespace {

using player::playback::domain::TrackDescriptor;
using player::playback::domain::TrackKind;

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

bool decodeTrack(
    const QVariantMap& map,
    TrackDescriptor* output,
    QString* errorMessage)
{
    QString type;
    if (!readRequiredPositiveId(map, &output->id, errorMessage)
        || !readRequiredType(map, &type, errorMessage)) {
        return false;
    }

    const auto kind = kindFromString(type);
    if (!kind.has_value()) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("track-list contains an unsupported track type '%1'.").arg(type));
        return false;
    }
    output->kind = *kind;

    return MpvMediaModelValueReader::optionalString(
               map, QStringLiteral("title"), &output->title, errorMessage)
        && MpvMediaModelValueReader::optionalString(
               map, QStringLiteral("lang"), &output->language, errorMessage)
        && MpvMediaModelValueReader::optionalString(
               map, QStringLiteral("codec"), &output->codec, errorMessage)
        && MpvMediaModelValueReader::optionalString(
               map, QStringLiteral("external-filename"), &output->externalFilename, errorMessage)
        && MpvMediaModelValueReader::optionalBool(
               map, QStringLiteral("selected"), &output->selected, errorMessage)
        && MpvMediaModelValueReader::optionalBool(
               map, QStringLiteral("default"), &output->defaultTrack, errorMessage)
        && MpvMediaModelValueReader::optionalBool(
               map, QStringLiteral("forced"), &output->forced, errorMessage)
        && MpvMediaModelValueReader::optionalBool(
               map, QStringLiteral("external"), &output->external, errorMessage)
        && MpvMediaModelValueReader::optionalBool(
               map, QStringLiteral("image"), &output->image, errorMessage)
        && MpvMediaModelValueReader::optionalBool(
               map, QStringLiteral("albumart"), &output->albumArt, errorMessage);
}

} // namespace

std::optional<QList<player::playback::domain::TrackDescriptor>> MpvTrackListDecoder::decode(
    const QVariant& node,
    QString* errorMessage)
{
    if (!node.isValid() || node.metaType().id() != QMetaType::QVariantList) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("track-list must be an mpv node array."));
        return std::nullopt;
    }

    const QVariantList list = node.toList();
    QList<TrackDescriptor> tracks;
    tracks.reserve(list.size());

    for (const QVariant& item : list) {
        if (item.metaType().id() != QMetaType::QVariantMap) {
            MpvMediaModelValueReader::setError(
                errorMessage,
                QStringLiteral("track-list contains a non-map entry."));
            return std::nullopt;
        }

        TrackDescriptor track;
        if (!decodeTrack(item.toMap(), &track, errorMessage)) {
            return std::nullopt;
        }
        tracks.append(std::move(track));
    }

    return tracks;
}

} // namespace player::playback::mpv
