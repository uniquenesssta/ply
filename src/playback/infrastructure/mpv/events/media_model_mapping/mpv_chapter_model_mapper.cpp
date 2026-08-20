#include "mpv_chapter_model_mapper.h"

#include "mpv_chapter_list_decoder.h"
#include "mpv_media_model_value_reader.h"
#include "playback/domain/events/chapter_event.h"

#include <QVariant>

#include <optional>
#include <utility>

namespace player::playback::mpv {

using namespace player::playback::domain;

std::optional<PlaybackEvent> MpvChapterModelMapper::map(
    const MpvPropertyValue& value,
    QString* errorMessage)
{
    bool unavailable = false;
    const QVariant* node = MpvMediaModelValueReader::nodeValue(
        value,
        &unavailable,
        errorMessage);
    if (unavailable) {
        return PlaybackEvent{ChapterListChangedEvent{}};
    }
    if (node == nullptr) {
        return std::nullopt;
    }

    auto chapters = MpvChapterListDecoder::decode(*node, errorMessage);
    if (!chapters.has_value()) {
        return std::nullopt;
    }
    return PlaybackEvent{ChapterListChangedEvent{std::move(*chapters)}};
}

} // namespace player::playback::mpv
