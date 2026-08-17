#include "playlist/domain/playlist_navigation.h"

#include "playlist/domain/playlist.h"

#include <algorithm>
#include <iterator>

namespace player::playlist::domain {

PlaylistNavigationDecision PlaylistNavigation::afterNaturalEnd(
    const Playlist& playlist) noexcept
{
    const auto currentId = playlist.currentId();
    const auto& entries = playlist.entries();
    if (!currentId.has_value() || entries.empty()) {
        return {};
    }

    if (playlist.repeatMode() == PlaylistRepeatMode::One) {
        return {PlaylistNavigationAction::ReloadCurrent, *currentId};
    }

    // Shuffle requires its own visited-order/history policy. Until that R7-08
    // sub-step exists, never silently fall back to linear advancement while
    // shuffle is enabled.
    if (playlist.shuffleEnabled()) {
        return {};
    }

    const auto current = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [currentId](const PlaylistEntry& entry) {
            return entry.id() == *currentId;
        });
    if (current == entries.cend()) {
        return {};
    }

    const auto next = std::next(current);
    if (next != entries.cend()) {
        return {PlaylistNavigationAction::SelectEntry, next->id()};
    }

    if (playlist.repeatMode() != PlaylistRepeatMode::All) {
        return {};
    }

    if (entries.size() == 1) {
        return {PlaylistNavigationAction::ReloadCurrent, *currentId};
    }

    return {PlaylistNavigationAction::SelectEntry, entries.front().id()};
}

} // namespace player::playlist::domain
