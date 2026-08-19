#include "tracks/application/subtitle_delay_controller.h"

#include "playback/domain/state/playback_lifecycle_state.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace player::tracks::application {
namespace {

using namespace player::playback::domain;

constexpr double kComparisonToleranceSeconds = 0.0005;

} // namespace

SubtitleDelayController::SubtitleDelayController(
    SubmitDelay submitDelay,
    QObject* parent)
    : QObject(parent)
    , submitDelay_(std::move(submitDelay))
{
}

bool SubtitleDelayController::available() const noexcept
{
    return available_;
}

bool SubtitleDelayController::pending() const noexcept
{
    return pendingTargetSeconds_.has_value();
}

double SubtitleDelayController::delaySeconds() const noexcept
{
    return delaySeconds_;
}

double SubtitleDelayController::pendingTargetSeconds() const noexcept
{
    return pendingTargetSeconds_.value_or(delaySeconds_);
}

int SubtitleDelayController::delayMilliseconds() const noexcept
{
    return toMilliseconds(delaySeconds_);
}

int SubtitleDelayController::pendingTargetMilliseconds() const noexcept
{
    return toMilliseconds(pendingTargetSeconds());
}

double SubtitleDelayController::minimumSeconds() const noexcept
{
    return kSubtitleDelayMinimumSeconds;
}

double SubtitleDelayController::maximumSeconds() const noexcept
{
    return kSubtitleDelayMaximumSeconds;
}

double SubtitleDelayController::stepSeconds() const noexcept
{
    return kSubtitleDelayStepSeconds;
}

bool SubtitleDelayController::setDelaySeconds(double seconds)
{
    if (!available_ || !std::isfinite(seconds)
        || seconds < kSubtitleDelayMinimumSeconds
        || seconds > kSubtitleDelayMaximumSeconds) {
        return false;
    }

    const double target = quantize(seconds);
    if (pendingTargetSeconds_.has_value()
        && nearlyEqual(*pendingTargetSeconds_, target)) {
        return true;
    }
    if (!pendingTargetSeconds_.has_value() && nearlyEqual(delaySeconds_, target)) {
        return true;
    }

    if (!submitDelay_ || !submitDelay_(SetSubtitleDelayCommand{target})) {
        return false;
    }

    pendingTargetSeconds_ = target;
    emit stateChanged();
    return true;
}

bool SubtitleDelayController::resetDelay()
{
    return setDelaySeconds(0.0);
}

void SubtitleDelayController::acceptSnapshot(const PlaybackSnapshot& snapshot)
{
    const bool generationChanged = snapshot.generation() != generation_;
    const auto confirmedDelay = snapshot.controls().subtitleDelaySeconds;
    const bool nextAvailable = snapshot.lifecycle() == PlaybackLifecycleState::Ready
        && snapshot.tracks().selectedSubtitleId.has_value()
        && confirmedDelay.has_value();
    const double nextDelay = confirmedDelay.value_or(0.0);

    bool changed = generationChanged
        || available_ != nextAvailable
        || !nearlyEqual(delaySeconds_, nextDelay);
    bool confirmed = false;

    if (generationChanged && pendingTargetSeconds_.has_value()) {
        pendingTargetSeconds_.reset();
        changed = true;
    }

    generation_ = snapshot.generation();
    available_ = nextAvailable;
    delaySeconds_ = nextDelay;

    if (pendingTargetSeconds_.has_value()
        && confirmedDelay.has_value()
        && nearlyEqual(*pendingTargetSeconds_, *confirmedDelay)) {
        pendingTargetSeconds_.reset();
        changed = true;
        confirmed = true;
    }

    if (changed) {
        emit stateChanged();
    }
    if (confirmed) {
        emit delayConfirmed(delayMilliseconds());
    }
}

bool SubtitleDelayController::rejectPendingDelay()
{
    if (!pendingTargetSeconds_.has_value()) {
        return false;
    }

    pendingTargetSeconds_.reset();
    emit stateChanged();
    return true;
}

double SubtitleDelayController::quantize(double seconds) noexcept
{
    const double clamped = std::clamp(
        seconds,
        kSubtitleDelayMinimumSeconds,
        kSubtitleDelayMaximumSeconds);
    return std::round(clamped / kSubtitleDelayStepSeconds)
        * kSubtitleDelayStepSeconds;
}

bool SubtitleDelayController::nearlyEqual(double lhs, double rhs) noexcept
{
    return std::abs(lhs - rhs) <= kComparisonToleranceSeconds;
}

int SubtitleDelayController::toMilliseconds(double seconds) noexcept
{
    return static_cast<int>(std::lround(seconds * 1000.0));
}

} // namespace player::tracks::application
