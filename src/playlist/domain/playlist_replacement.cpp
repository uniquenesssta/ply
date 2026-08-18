#include "playlist/domain/playlist_replacement.h"

#include <utility>

namespace player::playlist::domain {

PlaylistReplacement::PlaylistReplacement(
    std::vector<PlaylistEntry> entries,
    PlaylistEntryId currentId,
    quint64 nextEntryId) noexcept
    : entries_(std::move(entries))
    , currentId_(currentId)
    , nextEntryId_(nextEntryId)
{
}

const std::vector<PlaylistEntry>& PlaylistReplacement::entries() const noexcept
{
    return entries_;
}

PlaylistEntryId PlaylistReplacement::currentId() const noexcept
{
    return currentId_;
}

} // namespace player::playlist::domain
