#include "mpv_chapter_list_decoder.h"

#include "mpv_media_model_value_reader.h"

#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <cmath>
#include <utility>

namespace player::playback::mpv {

std::optional<QList<player::playback::domain::ChapterDescriptor>> MpvChapterListDecoder::decode(
    const QVariant& node,
    QString* errorMessage)
{
    using player::playback::domain::ChapterDescriptor;

    if (!node.isValid() || node.metaType().id() != QMetaType::QVariantList) {
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("chapter-list must be an mpv node array."));
        return std::nullopt;
    }

    const QVariantList list = node.toList();
    QList<ChapterDescriptor> chapters;
    chapters.reserve(list.size());

    for (qsizetype index = 0; index < list.size(); ++index) {
        const QVariant& item = list.at(index);
        if (item.metaType().id() != QMetaType::QVariantMap) {
            MpvMediaModelValueReader::setError(
                errorMessage,
                QStringLiteral("chapter-list contains a non-map entry."));
            return std::nullopt;
        }

        const QVariantMap map = item.toMap();
        const auto time = map.constFind(QStringLiteral("time"));
        if (time == map.cend() || !time->isValid()
            || !MpvMediaModelValueReader::isNumeric(*time)) {
            MpvMediaModelValueReader::setError(
                errorMessage,
                QStringLiteral("Chapter entry is missing a numeric 'time' field."));
            return std::nullopt;
        }

        bool converted = false;
        const double startSeconds = time->toDouble(&converted);
        if (!converted || !std::isfinite(startSeconds) || startSeconds < 0.0) {
            MpvMediaModelValueReader::setError(
                errorMessage,
                QStringLiteral("Chapter entry contains an invalid 'time' value."));
            return std::nullopt;
        }

        ChapterDescriptor chapter;
        chapter.index = index;
        chapter.startSeconds = startSeconds;
        if (!MpvMediaModelValueReader::optionalString(
                map,
                QStringLiteral("title"),
                &chapter.title,
                errorMessage)) {
            return std::nullopt;
        }
        chapters.append(std::move(chapter));
    }

    return chapters;
}

} // namespace player::playback::mpv
