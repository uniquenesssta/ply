#include "presentation/viewmodels/player/timeline/timeline_scrub_session.h"

#include <algorithm>
#include <cmath>

namespace player::presentation {

TimelineScrubPhase TimelineScrubSession::phase() const noexcept
{
    return phase_;
}

bool TimelineScrubSession::isActive() const noexcept
{
    return phase_ != TimelineScrubPhase::Idle;
}

bool TimelineScrubSession::isScrubbing() const noexcept
{
    return phase_ == TimelineScrubPhase::Scrubbing;
}

double TimelineScrubSession::previewNormalized() const noexcept
{
    return previewNormalized_;
}

player::playback::domain::MediaGeneration TimelineScrubSession::generation() const noexcept
{
    return generation_;
}

bool TimelineScrubSession::begin(
    player::playback::domain::MediaGeneration generation,
    double normalized) noexcept
{
    if (isActive() || !generation.isValid() || !isValidNormalized(normalized)) {
        return false;
    }

    generation_ = generation;
    previewNormalized_ = clampNormalized(normalized);
    phase_ = TimelineScrubPhase::Scrubbing;
    return true;
}

bool TimelineScrubSession::update(double normalized) noexcept
{
    if (!isScrubbing() || !isValidNormalized(normalized)) {
        return false;
    }

    previewNormalized_ = clampNormalized(normalized);
    return true;
}

std::optional<double> TimelineScrubSession::commit(double normalized) noexcept
{
    if (!isScrubbing() || !isValidNormalized(normalized)) {
        return std::nullopt;
    }

    const double committed = clampNormalized(normalized);
    reset();
    return committed;
}

bool TimelineScrubSession::cancel() noexcept
{
    if (!isActive()) {
        return false;
    }

    reset();
    return true;
}

bool TimelineScrubSession::cancelIfGenerationChanged(
    player::playback::domain::MediaGeneration generation) noexcept
{
    if (!isActive() || generation == generation_) {
        return false;
    }

    reset();
    return true;
}

bool TimelineScrubSession::isValidNormalized(double normalized) noexcept
{
    return std::isfinite(normalized);
}

double TimelineScrubSession::clampNormalized(double normalized) noexcept
{
    return std::clamp(normalized, 0.0, 1.0);
}

void TimelineScrubSession::reset() noexcept
{
    phase_ = TimelineScrubPhase::Idle;
    generation_ = {};
    previewNormalized_ = 0.0;
}

} // namespace player::presentation
