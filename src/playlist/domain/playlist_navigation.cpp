#include "playlist/domain/playlist_navigation.h"

#include "playlist/domain/playlist.h"
#include "playlist/domain/playlist_snapshot.h"

#include <algorithm>
#include <iterator>

namespace player::playlist::domain {

PlaylistNavigationCapabilities PlaylistNavigation::capabilities(
    const PlaylistSnapshot& snapshot) noexcept
{
    const std::optional<std::size_t> currentIndex = snapshot.currentIndex();
    if (!currentIndex.has_value()
        || *currentIndex >= snapshot.size()
        || snapshot.size() < 2) {
        return {};
    }

    if (snapshot.shuffleEnabled()) {
        // Shuffle currently owns only a forward cycle bag. Without an explicit
        // shuffle Previous history owner, advertising Previous here would claim
        // a capability the queue cannot execute deterministically.
        const bool canNext = !snapshot.shuffleCycleInitialized()
            || !snapshot.shuffleRemainingEntryIds().empty()
            || snapshot.repeatMode() == PlaylistRepeatMode::All;
        return {canNext, false};
    }

    const bool wrap = snapshot.repeatMode() == PlaylistRepeatMode::All;
    return {
        *currentIndex + 1 < snapshot.size() || wrap,
        *currentIndex > 0 || wrap,
    };
}

PlaylistNavigationDecision PlaylistNavigation::forManualNext(
    Playlist& playlist)
{
    const auto currentId = playlist.currentId();
    const auto& entries = playlist.entries();
    if (!currentId.has_value() || entries.size() < 2) {
        return {};
    }

    if (playlist.shuffleEnabled()) {
        const std::optional<PlaylistEntryId> next = playlist.previewNextShuffledId(
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

    if (playlist.repeatMode() == PlaylistRepeatMode::All) {
        return {PlaylistNavigationAction::SelectEntry, entries.front().id()};
    }

    // Repeat One affects natural EOF only. An explicit user Next must never
    // be converted into a reload of the current entry.
    return {};
}

PlaylistNavigationDecision PlaylistNavigation::forManualPrevious(
    const Playlist& playlist)
{
    const auto currentId = playlist.currentId();
    const auto& entries = playlist.entries();
    if (!currentId.has_value() || entries.size() < 2 || playlist.shuffleEnabled()) {
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

    if (current != entries.cbegin()) {
        return {PlaylistNavigationAction::SelectEntry, std::prev(current)->id()};
    }

    if (playlist.repeatMode() == PlaylistRepeatMode::All) {
        return {PlaylistNavigationAction::SelectEntry, entries.back().id()};
    }

    // Shuffle Previous requires explicit history ownership and must not be
    // inferred from visual order or the forward shuffle cycle bag.
    return {};
}

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

PlaylistNavigationDecision PlaylistNavigation::forCurrentRemoval(
    const Playlist& playlist,
    PlaylistEntryId currentId)
{
    const auto authoritativeCurrent = playlist.currentId();
    const auto& entries = playlist.entries();
    if (!currentId.isValid()
        || !authoritativeCurrent.has_value()
        || *authoritativeCurrent != currentId
        || entries.empty()) {
        return {};
    }

    const auto current = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [currentId](const PlaylistEntry& entry) {
            return entry.id() == currentId;
        });
    if (current == entries.cend()) {
        return {};
    }

    if (entries.size() == 1) {
        return {PlaylistNavigationAction::StopPlayback, PlaylistEntryId{}};
    }

    const auto next = std::next(current);
    if (next != entries.cend()) {
        return {PlaylistNavigationAction::SelectEntry, next->id()};
    }

    return {PlaylistNavigationAction::SelectEntry, std::prev(current)->id()};
}

} // namespace player::playlist::domain
