#include "playlist/domain/playlist_entry.h"

#include <utility>

namespace player::playlist::domain {

PlaylistEntry::PlaylistEntry(
    PlaylistEntryId id,
    media::domain::MediaSource source)
    : id_(id)
    , source_(std::move(source))
{
}

PlaylistEntryId PlaylistEntry::id() const noexcept
{
    return id_;
}

const media::domain::MediaSource& PlaylistEntry::source() const noexcept
{
    return source_;
}

} // namespace player::playlist::domain
