#include "playlist/application/playlist_controller.h"

#include "playlist/application/playlist_mutation.h"

#include <utility>
#include <vector>

namespace player::playlist::application {

PlaylistController::PlaylistController(
    domain::Playlist& playlist,
    PlaylistMutation& mutation,
    SubmitMediaLoad submitMediaLoad,
    QObject* parent)
    : QObject(parent)
    , playlist_(playlist)
    , mutation_(mutation)
    , submitMediaLoad_(std::move(submitMediaLoad))
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

    // R7-09 owns the next/stop policy for deleting the actively playing entry.
    // Until that policy exists, reject the operation rather than desynchronizing
    // Playlist current state from PlaybackSession's actually loaded media.
    if (playlist_.currentId().has_value() && *playlist_.currentId() == id) {
        return false;
    }

    if (!mutation_.remove(id)) {
        return false;
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
