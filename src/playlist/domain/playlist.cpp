#include "playlist/domain/playlist.h"

#include <QtGlobal>

#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>

namespace player::playlist::domain {

std::optional<PlaylistEntryId> Playlist::append(
    media::domain::MediaSource source)
{
    if (!source.isValid()) {
        return std::nullopt;
    }

    const std::optional<PlaylistEntryId> id = allocateEntryId();
    if (!id.has_value()) {
        return std::nullopt;
    }

    entries_.emplace_back(*id, std::move(source));
    shuffleState_.onEntryAdded(*id);
    return id;
}

std::optional<PlaylistEntryId> Playlist::insert(
    media::domain::MediaSource source,
    std::size_t targetIndex)
{
    if (!source.isValid() || targetIndex > entries_.size()) {
        return std::nullopt;
    }

    const std::optional<PlaylistEntryId> id = allocateEntryId();
    if (!id.has_value()) {
        return std::nullopt;
    }

    entries_.insert(
        entries_.begin() + static_cast<std::ptrdiff_t>(targetIndex),
        PlaylistEntry{*id, std::move(source)});
    shuffleState_.onEntryAdded(*id);
    return id;
}

bool Playlist::remove(PlaylistEntryId id)
{
    const auto iterator = std::find_if(
        entries_.begin(),
        entries_.end(),
        [id](const PlaylistEntry& entry) {
            return entry.id() == id;
        });

    if (iterator == entries_.end()) {
        return false;
    }

    shuffleState_.onEntryRemoved(id);
    if (currentId_.has_value() && *currentId_ == id) {
        currentId_.reset();
    }

    entries_.erase(iterator);
    return true;
}

bool Playlist::removeCurrentAndSelect(
    PlaylistEntryId id,
    PlaylistEntryId replacementId)
{
    if (!currentId_.has_value()
        || *currentId_ != id
        || !replacementId.isValid()
        || replacementId == id
        || find(replacementId) == nullptr) {
        return false;
    }

    const auto iterator = std::find_if(
        entries_.begin(),
        entries_.end(),
        [id](const PlaylistEntry& entry) {
            return entry.id() == id;
        });
    if (iterator == entries_.end()) {
        return false;
    }

    shuffleState_.onEntryRemoved(id);
    entries_.erase(iterator);
    currentId_ = replacementId;
    shuffleState_.onEntrySelected(replacementId);
    return true;
}

bool Playlist::move(PlaylistEntryId id, std::size_t targetIndex)
{
    if (targetIndex >= entries_.size()) {
        return false;
    }

    const auto iterator = std::find_if(
        entries_.begin(),
        entries_.end(),
        [id](const PlaylistEntry& entry) {
            return entry.id() == id;
        });
    if (iterator == entries_.end()) {
        return false;
    }

    const std::size_t sourceIndex = static_cast<std::size_t>(
        std::distance(entries_.begin(), iterator));
    if (sourceIndex == targetIndex) {
        return true;
    }

    PlaylistEntry entry = std::move(*iterator);
    entries_.erase(iterator);
    entries_.insert(
        entries_.begin() + static_cast<std::ptrdiff_t>(targetIndex),
        std::move(entry));
    return true;
}

bool Playlist::select(PlaylistEntryId id) noexcept
{
    if (find(id) == nullptr) {
        return false;
    }

    currentId_ = id;
    shuffleState_.onEntrySelected(id);
    return true;
}

std::optional<PlaylistReplacement> Playlist::prepareReplacement(
    const std::vector<media::domain::MediaSource>& sources) const
{
    if (sources.empty()) {
        return std::nullopt;
    }

    for (const media::domain::MediaSource& source : sources) {
        if (!source.isValid()) {
            return std::nullopt;
        }
    }

    std::vector<PlaylistEntry> replacementEntries;
    replacementEntries.reserve(sources.size());

    quint64 candidateNextId = nextEntryId_;
    for (const media::domain::MediaSource& source : sources) {
        if (candidateNextId == 0) {
            return std::nullopt;
        }

        const PlaylistEntryId id{candidateNextId};
        replacementEntries.emplace_back(id, source);

        if (candidateNextId == std::numeric_limits<quint64>::max()) {
            candidateNextId = 0;
        } else {
            ++candidateNextId;
        }
    }

    const PlaylistEntryId replacementCurrent = replacementEntries.front().id();
    return PlaylistReplacement{
        std::move(replacementEntries),
        replacementCurrent,
        candidateNextId,
    };
}

void Playlist::commitReplacement(PlaylistReplacement replacement) noexcept
{
    entries_ = std::move(replacement.entries_);
    currentId_ = replacement.currentId_;
    nextEntryId_ = replacement.nextEntryId_;
    shuffleState_.resetCycle();
}

void Playlist::clearCurrent() noexcept
{
    currentId_.reset();
}

void Playlist::clear() noexcept
{
    entries_.clear();
    currentId_.reset();
    shuffleState_.resetCycle();
}

bool Playlist::empty() const noexcept
{
    return entries_.empty();
}

std::size_t Playlist::size() const noexcept
{
    return entries_.size();
}

const std::vector<PlaylistEntry>& Playlist::entries() const noexcept
{
    return entries_;
}

const PlaylistEntry* Playlist::find(PlaylistEntryId id) const noexcept
{
    const auto iterator = std::find_if(
        entries_.cbegin(),
        entries_.cend(),
        [id](const PlaylistEntry& entry) {
            return entry.id() == id;
        });

    return iterator == entries_.cend() ? nullptr : &(*iterator);
}

PlaylistSnapshot Playlist::snapshot() const
{
    std::optional<std::size_t> currentIndex;
    if (currentId_.has_value()) {
        const auto current = std::find_if(
            entries_.cbegin(),
            entries_.cend(),
            [this](const PlaylistEntry& entry) {
                return entry.id() == *currentId_;
            });
        if (current != entries_.cend()) {
            currentIndex = static_cast<std::size_t>(
                std::distance(entries_.cbegin(), current));
        }
    }

    Q_ASSERT(!currentId_.has_value() || currentIndex.has_value());

    return PlaylistSnapshot{
        entries_,
        currentId_,
        currentIndex,
        repeatMode_,
        shuffleState_.enabled(),
        shuffleState_.cycleInitialized(),
        shuffleState_.remainingEntryIds(),
    };
}

std::optional<PlaylistEntryId> Playlist::currentId() const noexcept
{
    return currentId_;
}

const PlaylistEntry* Playlist::currentEntry() const noexcept
{
    return currentId_.has_value() ? find(*currentId_) : nullptr;
}

PlaylistRepeatMode Playlist::repeatMode() const noexcept
{
    return repeatMode_;
}

void Playlist::setRepeatMode(PlaylistRepeatMode mode) noexcept
{
    repeatMode_ = mode;
}

bool Playlist::shuffleEnabled() const noexcept
{
    return shuffleState_.enabled();
}

void Playlist::setShuffleEnabled(bool enabled) noexcept
{
    shuffleState_.setEnabled(enabled);
}

std::optional<PlaylistEntryId> Playlist::previewNextShuffledId(
    bool allowCycleRestart) const
{
    return shuffleState_.previewNext(entries_, currentId_, allowCycleRestart);
}

bool Playlist::selectNextShuffled(
    PlaylistEntryId id,
    bool allowCycleRestart) noexcept
{
    if (find(id) == nullptr
        || !shuffleState_.commitNextSelection(
            entries_,
            currentId_,
            id,
            allowCycleRestart)) {
        return false;
    }

    currentId_ = id;
    return true;
}

std::optional<PlaylistEntryId> Playlist::takeNextShuffledId(
    bool allowCycleRestart)
{
    return shuffleState_.takeNext(entries_, currentId_, allowCycleRestart);
}

std::optional<PlaylistEntryId> Playlist::allocateEntryId() noexcept
{
    if (nextEntryId_ == 0) {
        return std::nullopt;
    }

    const PlaylistEntryId id{nextEntryId_};
    if (nextEntryId_ == std::numeric_limits<quint64>::max()) {
        nextEntryId_ = 0;
    } else {
        ++nextEntryId_;
    }

    return id;
}

} // namespace player::playlist::domain
