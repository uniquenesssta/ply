#pragma once

#include <QString>
#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

struct VideoStreamInfo final
{
    std::optional<qint64> width;
    std::optional<qint64> height;
    std::optional<qint64> displayWidth;
    std::optional<qint64> displayHeight;
    std::optional<double> aspectRatio;
    std::optional<qint64> rotationDegrees;
    std::optional<QString> pixelFormat;

    bool operator==(const VideoStreamInfo&) const = default;
};

} // namespace player::playback::domain
