#pragma once

#include "playlist/domain/playlist_entry.h"
#include "playlist/domain/playlist_entry_id.h"

#include <optional>
#include <vector>

namespace player::playlist::domain {

class PlaylistShuffleState final
{
public:
    [[nodiscard]] bool enabled() const noexcept;
    [[nodiscard]] bool cycleInitialized() const noexcept;
    [[nodiscard]] const std::vector<PlaylistEntryId>& remainingEntryIds() const noexcept;
    void setEnabled(bool enabled) noexcept;
    void resetCycle() noexcept;

    void onEntryAdded(PlaylistEntryId id);
    void onEntryRemoved(PlaylistEntryId id) noexcept;
    void onEntrySelected(PlaylistEntryId id) noexcept;

    [[nodiscard]] std::optional<PlaylistEntryId> previewNext(
        const std::vector<PlaylistEntry>& entries,
        std::optional<PlaylistEntryId> currentId,
        bool allowCycleRestart) const;
    [[nodiscard]] bool commitNextSelection(
        const std::vector<PlaylistEntry>& entries,
        std::optional<PlaylistEntryId> currentId,
        PlaylistEntryId selectedId,
        bool allowCycleRestart);

    [[nodiscard]] std::optional<PlaylistEntryId> takeNext(
        const std::vector<PlaylistEntry>& entries,
        std::optional<PlaylistEntryId> currentId,
        bool allowCycleRestart);

private:
    void initializeCycle(
        const std::vector<PlaylistEntry>& entries,
        std::optional<PlaylistEntryId> currentId);
    [[nodiscard]] bool containsRemaining(PlaylistEntryId id) const noexcept;

    bool enabled_ = false;
    bool cycleInitialized_ = false;
    std::vector<PlaylistEntryId> remaining_;
};

} // namespace player::playlist::domain
