#include "playlist/domain/playlist_navigation.h"

#include "playlist/domain/playlist.h"

#include <algorithm>
#include <iterator>

namespace player::playlist::domain {

PlaylistNavigationDecision PlaylistNavigation::afterNaturalEnd(
    Playlist& playlist)
{
    const auto currentId = playlist.currentId();
    const auto& entries = playlist.entries();
    if (!currentId.has_value() || entries.empty()) {
        return {};
    }

    if (playlist.repeatMode() == PlaylistRepeatMode::One) {
        return {PlaylistNavigationAction::ReloadCurrent, *currentId};
    }

    if (entries.size() == 1) {
        if (playlist.repeatMode() == PlaylistRepeatMode::All) {
            return {PlaylistNavigationAction::ReloadCurrent, *currentId};
        }
        return {};
    }

    if (playlist.shuffleEnabled()) {
        const std::optional<PlaylistEntryId> next = playlist.takeNextShuffledId(
            playlist.repeatMode() == PlaylistRepeatMode::All);
        if (!next.has_value()) {
            return {};
        }
        return {PlaylistNavigationAction::SelectEntry, *next};
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

    return {PlaylistNavigationAction::SelectEntry, entries.front().id()};
}

PlaylistNavigationDecision PlaylistNavigation::afterPlaybackFailure(
    Playlist& playlist)
{
    const auto currentId = playlist.currentId();
    const auto& entries = playlist.entries();
    if (!currentId.has_value() || entries.size() < 2) {
        return {};
    }

    // Failure recovery deliberately does not restart repeat cycles. A broken
    // queue must converge instead of repeatedly reloading the same failed
    // entries under Repeat One or Repeat All.
    if (playlist.shuffleEnabled()) {
        const std::optional<PlaylistEntryId> next = playlist.takeNextShuffledId(false);
        if (!next.has_value()) {
            return {};
        }
        return {PlaylistNavigationAction::SelectEntry, *next};
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
    if (next == entries.cend()) {
        return {};
    }

    return {PlaylistNavigationAction::SelectEntry, next->id()};
}

} // namespace player::playlist::domain
