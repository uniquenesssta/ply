#include "playlist/application/playlist_auto_advance.h"

#include "playlist/application/playlist_controller.h"
#include "playlist/domain/playlist.h"
#include "playlist/domain/playlist_navigation.h"

namespace player::playlist::application {

PlaylistAutoAdvance::PlaylistAutoAdvance(
    domain::Playlist& playlist,
    PlaylistController& controller) noexcept
    : playlist_(playlist)
    , controller_(controller)
{
}

void PlaylistAutoAdvance::acceptPlaybackState(
    quint64 mediaGeneration,
    bool naturallyEnded)
{
    if (mediaGeneration == 0) {
        return;
    }

    if (handledTerminalGeneration_.has_value()
        && *handledTerminalGeneration_ != mediaGeneration) {
        handledTerminalGeneration_.reset();
    }

    if (!naturallyEnded) {
        return;
    }

    acceptTerminalState(mediaGeneration, TerminalReason::NaturalEnd);
}

void PlaylistAutoAdvance::acceptPlaybackFailure(quint64 mediaGeneration)
{
    acceptTerminalState(mediaGeneration, TerminalReason::Failure);
}

void PlaylistAutoAdvance::acceptTerminalState(
    quint64 mediaGeneration,
    TerminalReason reason)
{
    if (mediaGeneration == 0) {
        return;
    }

    if (handledTerminalGeneration_.has_value()
        && *handledTerminalGeneration_ != mediaGeneration) {
        handledTerminalGeneration_.reset();
    }

    if (handledTerminalGeneration_.has_value()
        && *handledTerminalGeneration_ == mediaGeneration) {
        return;
    }

    // Claim this media generation before submitting another load. A terminal
    // snapshot must never create a second automatic load, even if late Ended
    // and Failed projections for the same generation are both observed.
    handledTerminalGeneration_ = mediaGeneration;

    const domain::PlaylistNavigationDecision decision =
        reason == TerminalReason::NaturalEnd
        ? domain::PlaylistNavigation::afterNaturalEnd(playlist_)
        : domain::PlaylistNavigation::afterPlaybackFailure(playlist_);

    switch (decision.action) {
    case domain::PlaylistNavigationAction::None:
        return;
    case domain::PlaylistNavigationAction::SelectEntry:
        (void)controller_.selectEntry(decision.targetEntryId.value());
        return;
    case domain::PlaylistNavigationAction::ReloadCurrent:
        (void)controller_.reloadCurrentEntry();
        return;
    }
}

} // namespace player::playlist::application
