#include "playlist/domain/playlist_snapshot.h"

#include <utility>

namespace player::playlist::domain {

PlaylistSnapshot::PlaylistSnapshot(
    std::vector<PlaylistEntry> entries,
    std::optional<PlaylistEntryId> currentId,
    std::optional<std::size_t> currentIndex,
    PlaylistRepeatMode repeatMode,
    bool shuffleEnabled,
    bool shuffleCycleInitialized,
    std::vector<PlaylistEntryId> shuffleRemainingEntryIds)
    : entries_(std::move(entries))
    , currentId_(currentId)
    , currentIndex_(currentIndex)
    , repeatMode_(repeatMode)
    , shuffleEnabled_(shuffleEnabled)
    , shuffleCycleInitialized_(shuffleCycleInitialized)
    , shuffleRemainingEntryIds_(std::move(shuffleRemainingEntryIds))
{
}

bool PlaylistSnapshot::empty() const noexcept
{
    return entries_.empty();
}

std::size_t PlaylistSnapshot::size() const noexcept
{
    return entries_.size();
}

const std::vector<PlaylistEntry>& PlaylistSnapshot::entries() const noexcept
{
    return entries_;
}

std::optional<PlaylistEntryId> PlaylistSnapshot::currentId() const noexcept
{
    return currentId_;
}

std::optional<std::size_t> PlaylistSnapshot::currentIndex() const noexcept
{
    return currentIndex_;
}

PlaylistRepeatMode PlaylistSnapshot::repeatMode() const noexcept
{
    return repeatMode_;
}

bool PlaylistSnapshot::shuffleEnabled() const noexcept
{
    return shuffleEnabled_;
}

bool PlaylistSnapshot::shuffleCycleInitialized() const noexcept
{
    return shuffleCycleInitialized_;
}

const std::vector<PlaylistEntryId>&
PlaylistSnapshot::shuffleRemainingEntryIds() const noexcept
{
    return shuffleRemainingEntryIds_;
}

} // namespace player::playlist::domain
