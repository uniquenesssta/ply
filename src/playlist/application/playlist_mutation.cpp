#include "playlist/application/playlist_mutation.h"

#include "playlist/domain/playlist.h"

#include <utility>

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

std::optional<domain::PlaylistEntryId> PlaylistMutation::insert(
    const media::domain::MediaSource& source,
    std::size_t targetIndex)
{
    return playlist_.insert(source, targetIndex);
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

std::optional<domain::PlaylistReplacement> PlaylistMutation::prepareReplacement(
    const QList<media::domain::MediaSource>& sources) const
{
    if (sources.isEmpty()) {
        return std::nullopt;
    }

    std::vector<media::domain::MediaSource> sourceCopies;
    sourceCopies.reserve(static_cast<std::size_t>(sources.size()));
    for (const media::domain::MediaSource& source : sources) {
        sourceCopies.push_back(source);
    }

    return playlist_.prepareReplacement(sourceCopies);
}

void PlaylistMutation::commitReplacement(
    domain::PlaylistReplacement replacement) noexcept
{
    playlist_.commitReplacement(std::move(replacement));
}

void PlaylistMutation::clear() noexcept
{
    playlist_.clear();
}

void PlaylistMutation::setRepeatMode(domain::PlaylistRepeatMode mode) noexcept
{
    playlist_.setRepeatMode(mode);
}

void PlaylistMutation::setShuffleEnabled(bool enabled) noexcept
{
    playlist_.setShuffleEnabled(enabled);
}

} // namespace player::playlist::application
