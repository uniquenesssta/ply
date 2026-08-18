#pragma once

#include <QString>

namespace player::playback::domain {

// Loads an external subtitle file (SRT/ASS/...) into the current media.
// The loaded track must surface through the same TrackDescriptor model;
// no separate QML-side subtitle list is allowed.
struct LoadExternalSubtitleCommand final
{
    QString path;
};

} // namespace player::playback::domain
