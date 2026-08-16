#include "playlist/domain/playlist.h"

#include <algorithm>
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

    if (currentId_.has_value() && *currentId_ == id) {
        currentId_.reset();
    }

    entries_.erase(iterator);
    return true;
}

bool Playlist::select(PlaylistEntryId id) noexcept
{
    if (find(id) == nullptr) {
        return false;
    }

    currentId_ = id;
    return true;
}

void Playlist::clear() noexcept
{
    entries_.clear();
    currentId_.reset();
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
    return shuffleEnabled_;
}

void Playlist::setShuffleEnabled(bool enabled) noexcept
{
    shuffleEnabled_ = enabled;
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
