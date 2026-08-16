#pragma once

#include "media/domain/media_source.h"
#include "playlist/domain/playlist_entry_id.h"

namespace player::playlist::domain {

class PlaylistEntry final
{
public:
    PlaylistEntry(PlaylistEntryId id, media::domain::MediaSource source);

    [[nodiscard]] PlaylistEntryId id() const noexcept;
    [[nodiscard]] const media::domain::MediaSource& source() const noexcept;

private:
    PlaylistEntryId id_;
    media::domain::MediaSource source_;
};

} // namespace player::playlist::domain
