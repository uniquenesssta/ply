#pragma once

#include <QString>
#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

struct AudioStreamInfo final
{
    std::optional<QString> format;
    std::optional<qint64> sampleRate;
    std::optional<QString> channelLayout;
    std::optional<qint64> channelCount;
    std::optional<QString> humanReadableChannelLayout;

    bool operator==(const AudioStreamInfo&) const = default;
};

} // namespace player::playback::domain
