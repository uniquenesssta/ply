#include "playlist/presentation/playlist_entry_playback_state.h"

#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/domain/playlist_snapshot.h"

namespace player::playlist::presentation {

PlaylistEntryPlaybackState::PlaylistEntryPlaybackState(
    application::PlaylistController& controller,
    QObject* parent)
    : QObject(parent)
    , controller_(controller)
{
    connect(
        &controller_,
        &application::PlaylistController::playlistChanged,
        this,
        &PlaylistEntryPlaybackState::synchronizeWithPlaylist);
}

bool PlaylistEntryPlaybackState::isPendingLoading(
    domain::PlaylistEntryId entryId) const noexcept
{
    return pendingLoadingId_.has_value() && *pendingLoadingId_ == entryId;
}

bool PlaylistEntryPlaybackState::isUnavailable(
    domain::PlaylistEntryId entryId) const noexcept
{
    return unavailableEntryIds_.contains(entryId.value());
}

void PlaylistEntryPlaybackState::acceptPlaybackSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const std::optional<domain::PlaylistEntryId> currentId =
        controller_.snapshot().currentId();

    bool changed = false;
    switch (snapshot.lifecycle()) {
    case player::playback::domain::PlaybackLifecycleState::Opening:
        changed = setPendingLoading(currentId) || changed;
        if (currentId.has_value()) {
            changed = clearUnavailable(*currentId) || changed;
        }
        break;
    case player::playback::domain::PlaybackLifecycleState::Ready:
        changed = setPendingLoading(std::nullopt) || changed;
        if (currentId.has_value()) {
            changed = clearUnavailable(*currentId) || changed;
        }
        break;
    case player::playback::domain::PlaybackLifecycleState::Failed:
        changed = setPendingLoading(std::nullopt) || changed;
        if (currentId.has_value()) {
            changed = markUnavailable(*currentId) || changed;
        }
        break;
    case player::playback::domain::PlaybackLifecycleState::Empty:
    case player::playback::domain::PlaybackLifecycleState::Ended:
    case player::playback::domain::PlaybackLifecycleState::Closing:
        changed = setPendingLoading(std::nullopt) || changed;
        break;
    }

    if (changed) {
        emit entryStatesChanged();
    }
}

void PlaylistEntryPlaybackState::synchronizeWithPlaylist()
{
    const domain::PlaylistSnapshot snapshot = controller_.snapshot();
    QSet<quint64> existingEntryIds;
    existingEntryIds.reserve(static_cast<qsizetype>(snapshot.size()));
    for (const domain::PlaylistEntry& entry : snapshot.entries()) {
        existingEntryIds.insert(entry.id().value());
    }

    bool changed = false;
    const std::optional<domain::PlaylistEntryId> currentId = snapshot.currentId();
    if (pendingLoadingId_.has_value()
        && (!currentId.has_value()
            || *currentId != *pendingLoadingId_
            || !existingEntryIds.contains(pendingLoadingId_->value()))) {
        pendingLoadingId_.reset();
        changed = true;
    }

    const qsizetype unavailableCount = unavailableEntryIds_.size();
    unavailableEntryIds_.intersect(existingEntryIds);
    changed = unavailableEntryIds_.size() != unavailableCount || changed;

    if (changed) {
        emit entryStatesChanged();
    }
}

bool PlaylistEntryPlaybackState::setPendingLoading(
    std::optional<domain::PlaylistEntryId> entryId) noexcept
{
    if (pendingLoadingId_ == entryId) {
        return false;
    }

    pendingLoadingId_ = entryId;
    return true;
}

bool PlaylistEntryPlaybackState::clearUnavailable(
    domain::PlaylistEntryId entryId) noexcept
{
    return unavailableEntryIds_.remove(entryId.value()) > 0;
}

bool PlaylistEntryPlaybackState::markUnavailable(
    domain::PlaylistEntryId entryId) noexcept
{
    if (!entryId.isValid() || unavailableEntryIds_.contains(entryId.value())) {
        return false;
    }

    unavailableEntryIds_.insert(entryId.value());
    return true;
}

} // namespace player::playlist::presentation
