#pragma once

#include <QtGlobal>

#include <optional>

namespace player::playback::domain {

struct CacheStatus final
{
    std::optional<bool> beginningCached;
    std::optional<bool> endCached;
    std::optional<qint64> forwardBytes;
    std::optional<qint64> totalBytes;
    std::optional<double> cacheEndSeconds;
    std::optional<double> readerPositionSeconds;
    std::optional<double> durationSeconds;
    std::optional<qint64> rawInputRateBytesPerSecond;

    bool operator==(const CacheStatus&) const = default;
};

} // namespace player::playback::domain
