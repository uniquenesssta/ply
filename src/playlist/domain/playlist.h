#pragma once

#include "media/domain/media_source.h"
#include "playlist/domain/playlist_entry.h"
#include "playlist/domain/playlist_entry_id.h"
#include "playlist/domain/playlist_repeat_mode.h"
#include "playlist/domain/playlist_replacement.h"
#include "playlist/domain/playlist_shuffle_state.h"
#include "playlist/domain/playlist_snapshot.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace player::playlist::domain {

class Playlist final
{
public:
    [[nodiscard]] std::optional<PlaylistEntryId> append(
        media::domain::MediaSource source);
    [[nodiscard]] std::optional<PlaylistEntryId> insert(
        media::domain::MediaSource source,
        std::size_t targetIndex);
    [[nodiscard]] bool remove(PlaylistEntryId id);
    [[nodiscard]] bool removeCurrentAndSelect(
        PlaylistEntryId id,
        PlaylistEntryId replacementId);
    [[nodiscard]] bool move(PlaylistEntryId id, std::size_t targetIndex);
    [[nodiscard]] bool select(PlaylistEntryId id) noexcept;
    [[nodiscard]] std::optional<PlaylistReplacement> prepareReplacement(
        const std::vector<media::domain::MediaSource>& sources) const;
    void commitReplacement(PlaylistReplacement replacement) noexcept;
    void clearCurrent() noexcept;
    void clear() noexcept;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] const std::vector<PlaylistEntry>& entries() const noexcept;
    [[nodiscard]] const PlaylistEntry* find(PlaylistEntryId id) const noexcept;
    [[nodiscard]] PlaylistSnapshot snapshot() const;

    [[nodiscard]] std::optional<PlaylistEntryId> currentId() const noexcept;
    [[nodiscard]] const PlaylistEntry* currentEntry() const noexcept;

    [[nodiscard]] PlaylistRepeatMode repeatMode() const noexcept;
    void setRepeatMode(PlaylistRepeatMode mode) noexcept;

    [[nodiscard]] bool shuffleEnabled() const noexcept;
    void setShuffleEnabled(bool enabled) noexcept;
    [[nodiscard]] std::optional<PlaylistEntryId> previewNextShuffledId(
        bool allowCycleRestart) const;
    [[nodiscard]] bool selectNextShuffled(
        PlaylistEntryId id,
        bool allowCycleRestart) noexcept;
    [[nodiscard]] std::optional<PlaylistEntryId> takeNextShuffledId(
        bool allowCycleRestart);

private:
    [[nodiscard]] std::optional<PlaylistEntryId> allocateEntryId() noexcept;

    std::vector<PlaylistEntry> entries_;
    std::optional<PlaylistEntryId> currentId_;
    quint64 nextEntryId_ = 1;
    PlaylistRepeatMode repeatMode_ = PlaylistRepeatMode::Off;
    PlaylistShuffleState shuffleState_;
};

} // namespace player::playlist::domain
