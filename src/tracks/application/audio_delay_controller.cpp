#include "tracks/application/audio_delay_controller.h"

#include "playback/domain/state/playback_lifecycle_state.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace player::tracks::application {
namespace {

using namespace player::playback::domain;

constexpr double kComparisonToleranceSeconds = 0.0005;

} // namespace

AudioDelayController::AudioDelayController(
    SubmitDelay submitDelay,
    QObject* parent)
    : QObject(parent)
    , submitDelay_(std::move(submitDelay))
{
}

bool AudioDelayController::available() const noexcept
{
    return available_;
}

bool AudioDelayController::pending() const noexcept
{
    return pendingTargetSeconds_.has_value();
}

double AudioDelayController::delaySeconds() const noexcept
{
    return delaySeconds_;
}

double AudioDelayController::pendingTargetSeconds() const noexcept
{
    return pendingTargetSeconds_.value_or(delaySeconds_);
}

int AudioDelayController::delayMilliseconds() const noexcept
{
    return toMilliseconds(delaySeconds_);
}

int AudioDelayController::pendingTargetMilliseconds() const noexcept
{
    return toMilliseconds(pendingTargetSeconds());
}

double AudioDelayController::minimumSeconds() const noexcept
{
    return kAudioDelayMinimumSeconds;
}

double AudioDelayController::maximumSeconds() const noexcept
{
    return kAudioDelayMaximumSeconds;
}

double AudioDelayController::stepSeconds() const noexcept
{
    return kAudioDelayStepSeconds;
}

bool AudioDelayController::setDelaySeconds(double seconds)
{
    if (!available_ || !std::isfinite(seconds)
        || seconds < kAudioDelayMinimumSeconds
        || seconds > kAudioDelayMaximumSeconds) {
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

    if (!submitDelay_ || !submitDelay_(SetAudioDelayCommand{target})) {
        return false;
    }

    pendingTargetSeconds_ = target;
    emit stateChanged();
    return true;
}

bool AudioDelayController::resetDelay()
{
    return setDelaySeconds(0.0);
}

void AudioDelayController::acceptSnapshot(const PlaybackSnapshot& snapshot)
{
    const bool generationChanged = snapshot.generation() != generation_;
    const auto confirmedDelay = snapshot.controls().audioDelaySeconds;
    const bool nextAvailable = snapshot.lifecycle() == PlaybackLifecycleState::Ready
        && snapshot.tracks().selectedAudioId.has_value()
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

bool AudioDelayController::rejectPendingDelay()
{
    if (!pendingTargetSeconds_.has_value()) {
        return false;
    }

    pendingTargetSeconds_.reset();
    emit stateChanged();
    return true;
}

double AudioDelayController::quantize(double seconds) noexcept
{
    const double clamped = std::clamp(
        seconds,
        kAudioDelayMinimumSeconds,
        kAudioDelayMaximumSeconds);
    return std::round(clamped / kAudioDelayStepSeconds)
        * kAudioDelayStepSeconds;
}

bool AudioDelayController::nearlyEqual(double lhs, double rhs) noexcept
{
    return std::abs(lhs - rhs) <= kComparisonToleranceSeconds;
}

int AudioDelayController::toMilliseconds(double seconds) noexcept
{
    return static_cast<int>(std::lround(seconds * 1000.0));
}

} // namespace player::tracks::application
