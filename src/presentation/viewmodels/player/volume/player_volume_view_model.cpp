#include "presentation/viewmodels/player/volume/player_volume_view_model.h"

#include <algorithm>
#include <cmath>

namespace player::presentation {
namespace {

constexpr double kMaximumUiVolumePercent = 100.0;
constexpr double kPendingVolumeTolerancePercent = 0.5;
constexpr double kStateComparisonTolerance = 0.0001;

bool fuzzyEqual(double left, double right) noexcept
{
    return std::abs(left - right) <= kStateComparisonTolerance;
}

double clampUiVolumePercent(double percent) noexcept
{
    return std::clamp(percent, 0.0, kMaximumUiVolumePercent);
}

} // namespace

PlayerVolumeViewModel::PlayerVolumeViewModel(QObject* parent)
    : QObject(parent)
{
}

bool PlayerVolumeViewModel::canAdjustVolume() const noexcept
{
    return backendVolumePercent_.has_value();
}

bool PlayerVolumeViewModel::canToggleMute() const noexcept
{
    return backendMuted_.has_value();
}

double PlayerVolumeViewModel::normalizedVolume() const noexcept
{
    return clampUiVolumePercent(volumePercent()) / kMaximumUiVolumePercent;
}

double PlayerVolumeViewModel::volumePercent() const noexcept
{
    if (pendingVolumePercent_.has_value()) {
        return *pendingVolumePercent_;
    }
    return backendVolumePercent_.value_or(0.0);
}

bool PlayerVolumeViewModel::muted() const noexcept
{
    if (pendingMuted_.has_value()) {
        return *pendingMuted_;
    }
    return backendMuted_.value_or(false);
}

bool PlayerVolumeViewModel::volumePending() const noexcept
{
    return pendingVolumePercent_.has_value();
}

bool PlayerVolumeViewModel::mutePending() const noexcept
{
    return pendingMuted_.has_value();
}

bool PlayerVolumeViewModel::requestVolumeNormalized(double normalizedVolume)
{
    if (!canAdjustVolume() || !std::isfinite(normalizedVolume)) {
        return false;
    }

    const double requestedPercent =
        std::clamp(normalizedVolume, 0.0, 1.0) * kMaximumUiVolumePercent;
    if (fuzzyEqual(volumePercent(), requestedPercent)) {
        return true;
    }

    pendingVolumePercent_ = requestedPercent;
    emit stateChanged();
    emit volumeRequested(requestedPercent);
    return true;
}

bool PlayerVolumeViewModel::requestToggleMuted()
{
    if (!canToggleMute() || mutePending()) {
        return false;
    }

    pendingMuted_ = !muted();
    emit stateChanged();
    emit mutedRequested(*pendingMuted_);
    return true;
}

bool PlayerVolumeViewModel::rejectPendingVolume()
{
    if (!pendingVolumePercent_.has_value()) {
        return false;
    }

    pendingVolumePercent_.reset();
    emit stateChanged();
    return true;
}

bool PlayerVolumeViewModel::rejectPendingMute()
{
    if (!pendingMuted_.has_value()) {
        return false;
    }

    pendingMuted_.reset();
    emit stateChanged();
    return true;
}

void PlayerVolumeViewModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const bool oldCanAdjustVolume = canAdjustVolume();
    const bool oldCanToggleMute = canToggleMute();
    const double oldNormalizedVolume = normalizedVolume();
    const double oldVolumePercent = volumePercent();
    const bool oldMuted = muted();
    const bool oldVolumePending = volumePending();
    const bool oldMutePending = mutePending();

    const auto incomingVolume = snapshot.controls().volumePercent;
    if (incomingVolume.has_value()
        && std::isfinite(*incomingVolume)
        && *incomingVolume >= 0.0) {
        backendVolumePercent_ = *incomingVolume;
        if (pendingVolumePercent_.has_value()
            && std::abs(*incomingVolume - *pendingVolumePercent_)
                <= kPendingVolumeTolerancePercent) {
            pendingVolumePercent_.reset();
        }
    } else {
        backendVolumePercent_.reset();
        pendingVolumePercent_.reset();
    }

    const auto incomingMuted = snapshot.controls().muted;
    if (incomingMuted.has_value()) {
        backendMuted_ = *incomingMuted;
        if (pendingMuted_.has_value() && *pendingMuted_ == *incomingMuted) {
            pendingMuted_.reset();
        }
    } else {
        backendMuted_.reset();
        pendingMuted_.reset();
    }

    if (observableStateChanged(
            oldCanAdjustVolume,
            oldCanToggleMute,
            oldNormalizedVolume,
            oldVolumePercent,
            oldMuted,
            oldVolumePending,
            oldMutePending)) {
        emit stateChanged();
    }
}

bool PlayerVolumeViewModel::observableStateChanged(
    bool oldCanAdjustVolume,
    bool oldCanToggleMute,
    double oldNormalizedVolume,
    double oldVolumePercent,
    bool oldMuted,
    bool oldVolumePending,
    bool oldMutePending) const noexcept
{
    return oldCanAdjustVolume != canAdjustVolume()
        || oldCanToggleMute != canToggleMute()
        || !fuzzyEqual(oldNormalizedVolume, normalizedVolume())
        || !fuzzyEqual(oldVolumePercent, volumePercent())
        || oldMuted != muted()
        || oldVolumePending != volumePending()
        || oldMutePending != mutePending();
}

} // namespace player::presentation
