#pragma once

#include <QString>
#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

struct ChapterDescriptor final
{
    qsizetype index = 0;
    double startSeconds = 0.0;
    std::optional<QString> title;

    bool operator==(const ChapterDescriptor&) const = default;
};

} // namespace player::playback::domain
