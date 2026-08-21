#include "playback/domain/state/playback_chapter_state.h"

#include <cmath>

namespace player::playback::domain {

QList<ChapterDescriptor> PlaybackChapterState::validatedForDuration(
    const std::optional<double>& durationSeconds) const
{
    const bool hasValidDuration = durationSeconds.has_value()
        && std::isfinite(*durationSeconds)
        && *durationSeconds > 0.0;

    QList<ChapterDescriptor> validated;
    validated.reserve(chapters.size());
    for (const ChapterDescriptor& chapter : chapters) {
        if (chapter.index < 0
            || !std::isfinite(chapter.startSeconds)
            || chapter.startSeconds < 0.0
            || (hasValidDuration && chapter.startSeconds > *durationSeconds)) {
            continue;
        }
        validated.append(chapter);
    }
    return validated;
}

} // namespace player::playback::domain
