#pragma once

#include "playlist/domain/playlist_entry.h"
#include "playlist/domain/playlist_entry_id.h"
#include "playlist/domain/playlist_repeat_mode.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace player::playlist::domain {

class Playlist;

class PlaylistSnapshot final
{
public:
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] const std::vector<PlaylistEntry>& entries() const noexcept;

    [[nodiscard]] std::optional<PlaylistEntryId> currentId() const noexcept;
    [[nodiscard]] std::optional<std::size_t> currentIndex() const noexcept;

    [[nodiscard]] PlaylistRepeatMode repeatMode() const noexcept;
    [[nodiscard]] bool shuffleEnabled() const noexcept;
    [[nodiscard]] bool shuffleCycleInitialized() const noexcept;
    [[nodiscard]] const std::vector<PlaylistEntryId>& shuffleRemainingEntryIds() const noexcept;

private:
    friend class Playlist;

    PlaylistSnapshot(
        std::vector<PlaylistEntry> entries,
        std::optional<PlaylistEntryId> currentId,
        std::optional<std::size_t> currentIndex,
        PlaylistRepeatMode repeatMode,
        bool shuffleEnabled,
        bool shuffleCycleInitialized,
        std::vector<PlaylistEntryId> shuffleRemainingEntryIds);

    std::vector<PlaylistEntry> entries_;
    std::optional<PlaylistEntryId> currentId_;
    std::optional<std::size_t> currentIndex_;
    PlaylistRepeatMode repeatMode_ = PlaylistRepeatMode::Off;
    bool shuffleEnabled_ = false;
    bool shuffleCycleInitialized_ = false;
    std::vector<PlaylistEntryId> shuffleRemainingEntryIds_;
};

} // namespace player::playlist::domain
