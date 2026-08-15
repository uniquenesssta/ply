#include "presentation/viewmodels/player/timeline/timeline_seek_projection.h"

#include <algorithm>
#include <cmath>

namespace player::presentation {

bool TimelineSeekProjection::isPending() const noexcept
{
    return targetSeconds_.has_value();
}

player::playback::domain::MediaGeneration TimelineSeekProjection::generation() const noexcept
{
    return generation_;
}

std::optional<double> TimelineSeekProjection::targetSeconds() const noexcept
{
    return targetSeconds_;
}

bool TimelineSeekProjection::beginAbsolute(
    player::playback::domain::MediaGeneration generation,
    double targetSeconds,
    double durationSeconds) noexcept
{
    if (!generation.isValid()
        || !isFiniteNonNegative(targetSeconds)
        || !std::isfinite(durationSeconds)
        || durationSeconds <= 0.0) {
        return false;
    }

    generation_ = generation;
    targetSeconds_ = clampToDuration(targetSeconds, durationSeconds);
    return true;
}

std::optional<double> TimelineSeekProjection::nudgeRelative(
    player::playback::domain::MediaGeneration generation,
    double actualSeconds,
    double deltaSeconds,
    double durationSeconds) noexcept
{
    if (!generation.isValid()
        || !isFiniteNonNegative(actualSeconds)
        || !std::isfinite(deltaSeconds)
        || !std::isfinite(durationSeconds)
        || durationSeconds <= 0.0) {
        return std::nullopt;
    }

    if (isPending() && generation != generation_) {
        return std::nullopt;
    }

    const double baseSeconds = isPending()
        ? *targetSeconds_
        : clampToDuration(actualSeconds, durationSeconds);
    const double nextSeconds = clampToDuration(
        baseSeconds + deltaSeconds,
        durationSeconds);
    const double effectiveDeltaSeconds = nextSeconds - baseSeconds;

    if (std::abs(effectiveDeltaSeconds) <= 0.0000001) {
        return 0.0;
    }

    generation_ = generation;
    targetSeconds_ = nextSeconds;
    return effectiveDeltaSeconds;
}

bool TimelineSeekProjection::acknowledge(
    double actualSeconds,
    double toleranceSeconds) noexcept
{
    if (!isPending()
        || !isFiniteNonNegative(actualSeconds)
        || !std::isfinite(toleranceSeconds)
        || toleranceSeconds < 0.0
        || std::abs(actualSeconds - *targetSeconds_) > toleranceSeconds) {
        return false;
    }

    return clear();
}

bool TimelineSeekProjection::cancelIfGenerationChanged(
    player::playback::domain::MediaGeneration generation) noexcept
{
    if (!isPending() || generation == generation_) {
        return false;
    }

    return clear();
}

bool TimelineSeekProjection::clear() noexcept
{
    if (!isPending()) {
        return false;
    }

    generation_ = {};
    targetSeconds_.reset();
    return true;
}

bool TimelineSeekProjection::isFiniteNonNegative(double value) noexcept
{
    return std::isfinite(value) && value >= 0.0;
}

double TimelineSeekProjection::clampToDuration(
    double value,
    double durationSeconds) noexcept
{
    return std::clamp(value, 0.0, durationSeconds);
}

} // namespace player::presentation
