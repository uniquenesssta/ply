#pragma once

#include "playback/domain/models/chapter_descriptor.h"

#include <QList>

#include <optional>

class QString;
class QVariant;

namespace player::playback::mpv {

class MpvChapterListDecoder final
{
public:
    [[nodiscard]] static std::optional<QList<player::playback::domain::ChapterDescriptor>> decode(
        const QVariant& node,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
