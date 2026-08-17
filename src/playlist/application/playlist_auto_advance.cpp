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

    if (handledEndedGeneration_.has_value()
        && *handledEndedGeneration_ != mediaGeneration) {
        handledEndedGeneration_.reset();
    }

    if (!naturallyEnded) {
        return;
    }

    if (handledEndedGeneration_.has_value()
        && *handledEndedGeneration_ == mediaGeneration) {
        return;
    }

    // Claim this media generation before submitting another load. Late or
    // repeated snapshots from the same generation must never create a second
    // automatic load, even if that generation briefly leaves Ended again.
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
