#pragma once

#include "playback/domain/models/chapter_descriptor.h"

#include <QString>

namespace player::chapters::presentation {

class ChapterDisplayFormatter final
{
public:
    [[nodiscard]] static QString title(
        const player::playback::domain::ChapterDescriptor& chapter);
    [[nodiscard]] static QString time(double seconds);
};

} // namespace player::chapters::presentation
