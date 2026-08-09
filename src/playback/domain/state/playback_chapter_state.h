#pragma once

#include "playback/domain/models/chapter_descriptor.h"

#include <QList>

namespace player::playback::domain {

struct PlaybackChapterState final
{
    QList<ChapterDescriptor> chapters;

    bool operator==(const PlaybackChapterState&) const = default;
};

} // namespace player::playback::domain
