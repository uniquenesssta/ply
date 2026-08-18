#include "tracks/application/subtitle_delay_controller.h"

#include <cmath>

namespace player::tracks::application {

namespace {
constexpr double kStepSeconds = 0.5;
constexpr double kMinimumSeconds = -10.0;
constexpr double kMaximumSeconds = 10.0;
constexpr double kStateComparisonTolerance = 0.0001;

bool fuzzyEqual(double left, double right) noexcept
{
    return std::abs(left - right) <= kStateComparisonTolerance;
}
} // namespace

SubtitleDelayController::SubtitleDelayController(QObject* parent)
    : QObject(parent)
{
}

bool SubtitleDelayController::canAdjust() const noexcept
{
    return backendDelaySeconds_.has_value();
}

double SubtitleDelayController::delaySeconds() const noexcept
{
    if (pendingDelaySeconds_.has_value()) {
        return *pendingDelaySeconds_;
    }
    return backendDelaySeconds_.value_or(0.0);
}

QString SubtitleDelayController::delayText() const
{
    const double seconds = delaySeconds();
    if (std::abs(seconds) < kStateComparisonTolerance) {
        return QStringLiteral("0.0s");
    }
    return seconds > 0.0
        ? QStringLiteral("+%1s").arg(QString::number(seconds, 'f', 1))
        : QStringLiteral("%1s").arg(QString::number(seconds, 'f', 1));
}

double SubtitleDelayController::stepSeconds() const noexcept
{
    return kStepSeconds;
}

double SubtitleDelayController::minimumSeconds() const noexcept
{
    return kMinimumSeconds;
}

double SubtitleDelayController::maximumSeconds() const noexcept
{
    return kMaximumSeconds;
}

bool SubtitleDelayController::nudgeEarlier()
{
    return submit(std::max(kMinimumSeconds, delaySeconds() - kStepSeconds));
}

bool SubtitleDelayController::nudgeLater()
{
    return submit(std::min(kMaximumSeconds, delaySeconds() + kStepSeconds));
}

bool SubtitleDelayController::reset()
{
    return submit(0.0);
}

bool SubtitleDelayController::requestSet(double seconds)
{
    if (!std::isfinite(seconds)) {
        return false;
    }
    return submit(std::clamp(seconds, kMinimumSeconds, kMaximumSeconds));
}

bool SubtitleDelayController::rejectPending()
{
    if (!pendingDelaySeconds_.has_value()) {
        return false;
    }

    pendingDelaySeconds_.reset();
    emit stateChanged();
    return true;
}

void SubtitleDelayController::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const bool oldCanAdjust = canAdjust();
    const double oldDelay = delaySeconds();
    const bool oldPending = pendingDelaySeconds_.has_value();

    const auto incoming = snapshot.controls().subtitleDelaySeconds;
    if (incoming.has_value() && std::isfinite(*incoming)) {
        backendDelaySeconds_ = *incoming;
        // Any authoritative update clears the optimistic pending value:
        // the snapshot is the single truth for the displayed delay.
        pendingDelaySeconds_.reset();
    } else {
        backendDelaySeconds_.reset();
        pendingDelaySeconds_.reset();
    }

    if (oldCanAdjust != canAdjust()
        || !fuzzyEqual(oldDelay, delaySeconds())
        || oldPending != pendingDelaySeconds_.has_value()) {
        emit stateChanged();
    }
}

bool SubtitleDelayController::submit(double seconds)
{
    if (!canAdjust()) {
        return false;
    }

    if (fuzzyEqual(delaySeconds(), seconds)) {
        return true;
    }

    pendingDelaySeconds_ = seconds;
    emit stateChanged();
    emit subtitleDelayRequested(seconds);
    return true;
}

} // namespace player::tracks::application
