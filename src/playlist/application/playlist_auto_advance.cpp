#include "playlist/application/playlist_auto_advance.h"

#include "playlist/application/playlist_advance_arbiter.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/domain/playlist.h"
#include "playlist/domain/playlist_navigation.h"

namespace player::playlist::application {

PlaylistAutoAdvance::PlaylistAutoAdvance(
    domain::Playlist& playlist,
    PlaylistController& controller,
    PlaylistAdvanceArbiter& advanceArbiter) noexcept
    : playlist_(playlist)
    , controller_(controller)
    , advanceArbiter_(advanceArbiter)
{
}

void PlaylistAutoAdvance::acceptPlaybackState(
    quint64 mediaGeneration,
    bool naturallyEnded)
{
    advanceArbiter_.observeGeneration(mediaGeneration);
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
    if (!advanceArbiter_.tryClaimTerminal(mediaGeneration)) {
        return;
    }

    const domain::PlaylistNavigationDecision decision =
        reason == TerminalReason::NaturalEnd
        ? domain::PlaylistNavigation::afterNaturalEnd(playlist_)
        : domain::PlaylistNavigation::afterPlaybackFailure(playlist_);

    switch (decision.action) {
    case domain::PlaylistNavigationAction::None:
    case domain::PlaylistNavigationAction::StopPlayback:
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
