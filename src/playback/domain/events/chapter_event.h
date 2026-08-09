#pragma once

#include "playback/domain/models/chapter_descriptor.h"

#include <QList>

namespace player::playback::domain {

struct ChapterListChangedEvent final
{
    QList<ChapterDescriptor> chapters;
};

} // namespace player::playback::domain
