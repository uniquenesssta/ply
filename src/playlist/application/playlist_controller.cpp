#include "playlist/application/playlist_controller.h"

#include "playlist/application/playlist_mutation.h"
#include "playlist/domain/playlist_navigation.h"

#include <utility>
#include <vector>

namespace player::playlist::application {

PlaylistController::PlaylistController(
    domain::Playlist& playlist,
    PlaylistMutation& mutation,
    SubmitMediaLoad submitMediaLoad,
    QObject* parent)
    : PlaylistController(
        playlist,
        mutation,
        std::move(submitMediaLoad),
        SubmitMediaStop{},
        parent)
{
}

PlaylistController::PlaylistController(
    domain::Playlist& playlist,
    PlaylistMutation& mutation,
    SubmitMediaLoad submitMediaLoad,
    SubmitMediaStop submitMediaStop,
    QObject* parent)
    : QObject(parent)
    , playlist_(playlist)
    , mutation_(mutation)
    , submitMediaLoad_(std::move(submitMediaLoad))
    , submitMediaStop_(std::move(submitMediaStop))
{
}

const domain::Playlist& PlaylistController::playlist() const noexcept
{
    return playlist_;
}

bool PlaylistController::openSource(const media::domain::MediaSource& source)
{
    return openSources(QList<media::domain::MediaSource>{source});
}

bool PlaylistController::openSources(
    const QList<media::domain::MediaSource>& sources)
{
    if (!submitMediaLoad_) {
        return false;
    }

    const std::optional<domain::PlaylistEntryId> previousCurrent = playlist_.currentId();
    const std::optional<std::vector<domain::PlaylistEntryId>> appendedIds =
        mutation_.appendAll(sources);
    if (!appendedIds.has_value() || appendedIds->empty()) {
        return false;
    }

    const domain::PlaylistEntryId firstId = appendedIds->front();
    if (!playlist_.select(firstId)) {
        mutation_.rollbackAppend(*appendedIds);
        restoreCurrent(previousCurrent);
        return false;
    }

    const domain::PlaylistEntry* entry = playlist_.currentEntry();
    if (entry == nullptr || !submitLoadForEntry(*entry)) {
        mutation_.rollbackAppend(*appendedIds);
        restoreCurrent(previousCurrent);
        return false;
    }

    emit playlistChanged();
    return true;
}

bool PlaylistController::reloadCurrentEntry()
{
    const domain::PlaylistEntry* entry = playlist_.currentEntry();
    return entry != nullptr && submitLoadForEntry(*entry);
}

bool PlaylistController::removeEntry(quint64 entryId)
{
    const domain::PlaylistEntryId id{entryId};
    if (!id.isValid() || playlist_.find(id) == nullptr) {
        return false;
    }

    const std::optional<domain::PlaylistEntryId> currentId = playlist_.currentId();
    if (!currentId.has_value() || *currentId != id) {
        if (!mutation_.remove(id)) {
            return false;
        }

        emit playlistChanged();
        return true;
    }

    const domain::PlaylistNavigationDecision decision =
        domain::PlaylistNavigation::forCurrentRemoval(playlist_, id);
    switch (decision.action) {
    case domain::PlaylistNavigationAction::None:
    case domain::PlaylistNavigationAction::ReloadCurrent:
        return false;
    case domain::PlaylistNavigationAction::StopPlayback:
        if (!submitMediaStop_ || !submitMediaStop_()) {
            return false;
        }
        if (!mutation_.remove(id)) {
            return false;
        }
        break;
    case domain::PlaylistNavigationAction::SelectEntry: {
        const domain::PlaylistEntry* replacement = playlist_.find(decision.targetEntryId);
        if (replacement == nullptr || !submitLoadForEntry(*replacement)) {
            return false;
        }
        if (!mutation_.removeCurrentAndSelect(id, decision.targetEntryId)) {
            return false;
        }
        break;
    }
    }

    emit playlistChanged();
    return true;
}

bool PlaylistController::moveEntry(quint64 entryId, int targetIndex)
{
    const domain::PlaylistEntryId id{entryId};
    if (!id.isValid() || targetIndex < 0) {
        return false;
    }

    if (!mutation_.move(id, static_cast<std::size_t>(targetIndex))) {
        return false;
    }

    emit playlistChanged();
    return true;
}

bool PlaylistController::selectEntry(quint64 entryId)
{
    const domain::PlaylistEntryId id{entryId};
    const domain::PlaylistEntry* entry = playlist_.find(id);
    if (!id.isValid() || entry == nullptr || !submitMediaLoad_) {
        return false;
    }

    const std::optional<domain::PlaylistEntryId> previousCurrent = playlist_.currentId();
    if (previousCurrent.has_value() && *previousCurrent == id) {
        return true;
    }

    // Submit first so an immediate command-bus rejection cannot mutate Playlist
    // current/shuffle history. The entry was resolved above and remains owned by
    // Playlist for the duration of this synchronous call.
    if (!submitLoadForEntry(*entry)) {
        return false;
    }

    if (!playlist_.select(id)) {
        return false;
    }

    emit playlistChanged();
    return true;
}

bool PlaylistController::submitLoadForEntry(const domain::PlaylistEntry& entry)
{
    return submitMediaLoad_ && submitMediaLoad_(entry.source());
}

void PlaylistController::restoreCurrent(
    std::optional<domain::PlaylistEntryId> previousCurrent) noexcept
{
    if (previousCurrent.has_value() && playlist_.select(*previousCurrent)) {
        return;
    }

    playlist_.clearCurrent();
}

} // namespace player::playlist::application
