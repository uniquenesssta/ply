#include "playlist/application/playlist_advance_arbiter.h"

namespace player::playlist::application {

void PlaylistAdvanceArbiter::observeGeneration(quint64 mediaGeneration) noexcept
{
    (void)acceptGeneration(mediaGeneration);
}

bool PlaylistAdvanceArbiter::tryClaimTerminal(quint64 mediaGeneration) noexcept
{
    if (!acceptGeneration(mediaGeneration)) {
        return false;
    }

    if ((suppressedGeneration_.has_value()
         && *suppressedGeneration_ == mediaGeneration)
        || (claimedGeneration_.has_value()
            && *claimedGeneration_ == mediaGeneration)) {
        return false;
    }

    claimedGeneration_ = mediaGeneration;
    manualClaimActive_ = false;
    return true;
}

bool PlaylistAdvanceArbiter::tryClaimManualNavigation() noexcept
{
    if (!observedGeneration_.has_value()) {
        return true;
    }

    const quint64 mediaGeneration = *observedGeneration_;
    if ((suppressedGeneration_.has_value()
         && *suppressedGeneration_ == mediaGeneration)
        || (claimedGeneration_.has_value()
            && *claimedGeneration_ == mediaGeneration)) {
        return false;
    }

    claimedGeneration_ = mediaGeneration;
    manualClaimActive_ = true;
    return true;
}

void PlaylistAdvanceArbiter::releaseManualNavigationClaim() noexcept
{
    if (!manualClaimActive_) {
        return;
    }

    if (observedGeneration_.has_value()
        && claimedGeneration_.has_value()
        && *claimedGeneration_ == *observedGeneration_) {
        claimedGeneration_.reset();
    }
    manualClaimActive_ = false;
}

void PlaylistAdvanceArbiter::suppressObservedGeneration() noexcept
{
    if (!observedGeneration_.has_value()) {
        return;
    }

    suppressedGeneration_ = observedGeneration_;
    claimedGeneration_ = observedGeneration_;
    manualClaimActive_ = false;
}

bool PlaylistAdvanceArbiter::acceptGeneration(quint64 mediaGeneration) noexcept
{
    if (mediaGeneration == 0) {
        return false;
    }

    if (!observedGeneration_.has_value()) {
        observedGeneration_ = mediaGeneration;
        return true;
    }

    if (mediaGeneration < *observedGeneration_) {
        return false;
    }

    if (mediaGeneration > *observedGeneration_) {
        observedGeneration_ = mediaGeneration;
        claimedGeneration_.reset();
        suppressedGeneration_.reset();
        manualClaimActive_ = false;
    }

    return true;
}

} // namespace player::playlist::application
