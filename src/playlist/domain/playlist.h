#pragma once

#include "media/domain/media_source.h"
#include "playlist/domain/playlist_entry.h"
#include "playlist/domain/playlist_entry_id.h"
#include "playlist/domain/playlist_repeat_mode.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace player::playlist::domain {

class Playlist final
{
public:
    [[nodiscard]] std::optional<PlaylistEntryId> append(
        media::domain::MediaSource source);
    [[nodiscard]] bool remove(PlaylistEntryId id);
    [[nodiscard]] bool select(PlaylistEntryId id) noexcept;
    void clear() noexcept;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] const std::vector<PlaylistEntry>& entries() const noexcept;
    [[nodiscard]] const PlaylistEntry* find(PlaylistEntryId id) const noexcept;

    [[nodiscard]] std::optional<PlaylistEntryId> currentId() const noexcept;
    [[nodiscard]] const PlaylistEntry* currentEntry() const noexcept;

    [[nodiscard]] PlaylistRepeatMode repeatMode() const noexcept;
    void setRepeatMode(PlaylistRepeatMode mode) noexcept;

    [[nodiscard]] bool shuffleEnabled() const noexcept;
    void setShuffleEnabled(bool enabled) noexcept;

private:
    [[nodiscard]] std::optional<PlaylistEntryId> allocateEntryId() noexcept;

    std::vector<PlaylistEntry> entries_;
    std::optional<PlaylistEntryId> currentId_;
    quint64 nextEntryId_ = 1;
    PlaylistRepeatMode repeatMode_ = PlaylistRepeatMode::Off;
    bool shuffleEnabled_ = false;
};

} // namespace player::playlist::domain
