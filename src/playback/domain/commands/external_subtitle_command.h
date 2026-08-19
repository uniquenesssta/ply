#pragma once

#include <QString>

namespace player::playback::domain {

struct AddExternalSubtitleCommand final
{
    QString source;
};

} // namespace player::playback::domain
