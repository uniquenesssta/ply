#pragma once

#include <QString>

namespace player::playback::domain {

struct LoadMediaCommand final
{
    QString source;
};

} // namespace player::playback::domain
