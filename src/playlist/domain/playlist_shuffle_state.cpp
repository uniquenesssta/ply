#include "playlist/domain/playlist_shuffle_state.h"

#include <QRandomGenerator>

#include <algorithm>
#include <cstddef>

namespace player::playlist::domain {

bool PlaylistShuffleState::enabled() const noexcept
{
    return enabled_;
}

bool PlaylistShuffleState::cycleInitialized() const noexcept
{
    return cycleInitialized_;
}

const std::vector<PlaylistEntryId>& PlaylistShuffleState::remainingEntryIds() const noexcept
{
    return remaining_;
}

void PlaylistShuffleState::setEnabled(bool enabled) noexcept
{
    if (enabled_ == enabled) {
        return;
    }

    enabled_ = enabled;
    resetCycle();
}

void PlaylistShuffleState::resetCycle() noexcept
{
    cycleInitialized_ = false;
    remaining_.clear();
}

void PlaylistShuffleState::onEntryAdded(PlaylistEntryId id)
{
    if (!enabled_ || !cycleInitialized_ || !id.isValid() || containsRemaining(id)) {
        return;
    }

    remaining_.push_back(id);
}

void PlaylistShuffleState::onEntryRemoved(PlaylistEntryId id) noexcept
{
    if (!cycleInitialized_) {
        return;
    }

    std::erase(remaining_, id);
}

void PlaylistShuffleState::onEntrySelected(PlaylistEntryId id) noexcept
{
    if (!enabled_ || !cycleInitialized_) {
        return;
    }

    std::erase(remaining_, id);
}

std::optional<PlaylistEntryId> PlaylistShuffleState::takeNext(
    const std::vector<PlaylistEntry>& entries,
    std::optional<PlaylistEntryId> currentId,
    bool allowCycleRestart)
{
    if (!enabled_ || !currentId.has_value() || entries.empty()) {
        return std::nullopt;
    }

    if (!cycleInitialized_) {
        initializeCycle(entries, currentId);
    }

    if (remaining_.empty()) {
        if (!allowCycleRestart) {
            return std::nullopt;
        }

        initializeCycle(entries, currentId);
        if (remaining_.empty()) {
            return std::nullopt;
        }
    }

    const quint64 randomValue = QRandomGenerator::global()->generate64();
    const std::size_t index = static_cast<std::size_t>(
        randomValue % static_cast<quint64>(remaining_.size()));
    return remaining_.at(index);
}

void PlaylistShuffleState::initializeCycle(
    const std::vector<PlaylistEntry>& entries,
    std::optional<PlaylistEntryId> currentId)
{
    remaining_.clear();
    remaining_.reserve(entries.size());

    for (const PlaylistEntry& entry : entries) {
        if (currentId.has_value() && entry.id() == *currentId) {
            continue;
        }
        remaining_.push_back(entry.id());
    }

    cycleInitialized_ = true;
}

bool PlaylistShuffleState::containsRemaining(PlaylistEntryId id) const noexcept
{
    return std::find(remaining_.cbegin(), remaining_.cend(), id) != remaining_.cend();
}

} // namespace player::playlist::domain
