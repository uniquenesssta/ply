#include "playlist/application/playlist_mutation.h"

#include "playlist/domain/playlist.h"

namespace player::playlist::application {

PlaylistMutation::PlaylistMutation(domain::Playlist& playlist) noexcept
    : playlist_(playlist)
{
}

std::optional<std::vector<domain::PlaylistEntryId>> PlaylistMutation::appendAll(
    const QList<media::domain::MediaSource>& sources)
{
    if (sources.isEmpty()) {
        return std::nullopt;
    }

    std::vector<domain::PlaylistEntryId> appendedIds;
    appendedIds.reserve(static_cast<std::size_t>(sources.size()));

    for (const media::domain::MediaSource& source : sources) {
        const std::optional<domain::PlaylistEntryId> id = playlist_.append(source);
        if (!id.has_value()) {
            rollbackAppend(appendedIds);
            return std::nullopt;
        }
        appendedIds.push_back(*id);
    }

    return appendedIds;
}

void PlaylistMutation::rollbackAppend(
    const std::vector<domain::PlaylistEntryId>& appendedIds)
{
    for (auto iterator = appendedIds.rbegin(); iterator != appendedIds.rend(); ++iterator) {
        (void)playlist_.remove(*iterator);
    }
}

bool PlaylistMutation::remove(domain::PlaylistEntryId id)
{
    return playlist_.remove(id);
}

bool PlaylistMutation::removeCurrentAndSelect(
    domain::PlaylistEntryId id,
    domain::PlaylistEntryId replacementId)
{
    return playlist_.removeCurrentAndSelect(id, replacementId);
}

bool PlaylistMutation::move(domain::PlaylistEntryId id, std::size_t targetIndex)
{
    return playlist_.move(id, targetIndex);
}

} // namespace player::playlist::application
