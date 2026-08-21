#pragma once

#include "playback/domain/models/chapter_descriptor.h"

#include <QList>

#include <optional>

namespace player::playback::domain {

struct PlaybackChapterState final
{
    QList<ChapterDescriptor> chapters;

    [[nodiscard]] QList<ChapterDescriptor> validatedForDuration(
        const std::optional<double>& durationSeconds) const;

    bool operator==(const PlaybackChapterState&) const = default;
};

} // namespace player::playback::domain
