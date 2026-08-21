#include "chapters/presentation/chapter_display_formatter.h"

#include <QChar>
#include <QtGlobal>

#include <cmath>

namespace player::chapters::presentation {

QString ChapterDisplayFormatter::title(
    const player::playback::domain::ChapterDescriptor& chapter)
{
    if (chapter.title.has_value() && !chapter.title->isEmpty()) {
        return *chapter.title;
    }
    return QStringLiteral("Chapter %1").arg(static_cast<qlonglong>(chapter.index + 1));
}

QString ChapterDisplayFormatter::time(double seconds)
{
    if (!std::isfinite(seconds) || seconds < 0.0) {
        return QStringLiteral("--:--:--");
    }

    const qint64 totalSeconds = static_cast<qint64>(std::floor(seconds));
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 remainingSeconds = totalSeconds % 60;
    return QStringLiteral("%1:%2:%3")
        .arg(hours, 2, 10, QChar(u'0'))
        .arg(minutes, 2, 10, QChar(u'0'))
        .arg(remainingSeconds, 2, 10, QChar(u'0'));
}

} // namespace player::chapters::presentation
