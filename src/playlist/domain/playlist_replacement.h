#pragma once

#include "playlist/domain/playlist_entry.h"
#include "playlist/domain/playlist_entry_id.h"

#include <QtGlobal>

#include <vector>

namespace player::playlist::domain {

class Playlist;

class PlaylistReplacement final
{
public:
    [[nodiscard]] const std::vector<PlaylistEntry>& entries() const noexcept;
    [[nodiscard]] PlaylistEntryId currentId() const noexcept;

private:
    friend class Playlist;

    PlaylistReplacement(
        std::vector<PlaylistEntry> entries,
        PlaylistEntryId currentId,
        quint64 nextEntryId) noexcept;

    std::vector<PlaylistEntry> entries_;
    PlaylistEntryId currentId_;
    quint64 nextEntryId_ = 0;
};

} // namespace player::playlist::domain
