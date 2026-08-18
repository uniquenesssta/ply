#include "mpv_track_model_mapper.h"

#include "mpv_media_model_value_reader.h"
#include "mpv_track_list_decoder.h"
#include "playback/domain/events/track_event.h"

#include <QMetaType>
#include <QString>
#include <QVariant>

#include <optional>
#include <utility>

namespace player::playback::mpv {
namespace {

using namespace player::playback::domain;

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

    auto tracks = MpvTrackListDecoder::decode(*node, errorMessage);
    if (!tracks.has_value()) {
        return std::nullopt;
    }
    return PlaybackEvent{TrackListChangedEvent{std::move(*tracks)}};
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
