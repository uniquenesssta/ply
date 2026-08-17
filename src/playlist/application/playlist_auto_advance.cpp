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

    if (!naturallyEnded) {
        if (handledEndedGeneration_.has_value()
            && *handledEndedGeneration_ == mediaGeneration) {
            handledEndedGeneration_.reset();
        }
        return;
    }

    if (handledEndedGeneration_.has_value()
        && *handledEndedGeneration_ == mediaGeneration) {
        return;
    }

    // Claim this EOF generation before submitting another load. StatePublisher
    // can publish more than one Ended snapshot before the next generation is
    // visible, and those repeats must never create duplicate loads.
    handledEndedGeneration_ = mediaGeneration;

    const domain::PlaylistNavigationDecision decision =
        domain::PlaylistNavigation::afterNaturalEnd(playlist_);
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
