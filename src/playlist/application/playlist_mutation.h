#pragma once

#include "media/domain/media_source.h"
#include "playlist/domain/playlist_entry_id.h"
#include "playlist/domain/playlist_repeat_mode.h"
#include "playlist/domain/playlist_replacement.h"

#include <QList>

#include <cstddef>
#include <optional>
#include <vector>

namespace player::playlist::domain {
class Playlist;
}

namespace player::playlist::application {

class PlaylistMutation final
{
public:
    explicit PlaylistMutation(domain::Playlist& playlist) noexcept;

    [[nodiscard]] std::optional<std::vector<domain::PlaylistEntryId>> appendAll(
        const QList<media::domain::MediaSource>& sources);
    void rollbackAppend(const std::vector<domain::PlaylistEntryId>& appendedIds);

    [[nodiscard]] std::optional<domain::PlaylistEntryId> insert(
        const media::domain::MediaSource& source,
        std::size_t targetIndex);
    [[nodiscard]] bool remove(domain::PlaylistEntryId id);
    [[nodiscard]] bool removeCurrentAndSelect(
        domain::PlaylistEntryId id,
        domain::PlaylistEntryId replacementId);
    [[nodiscard]] bool move(domain::PlaylistEntryId id, std::size_t targetIndex);

    [[nodiscard]] std::optional<domain::PlaylistReplacement> prepareReplacement(
        const QList<media::domain::MediaSource>& sources) const;
    void commitReplacement(domain::PlaylistReplacement replacement) noexcept;
    void clear() noexcept;
    void setRepeatMode(domain::PlaylistRepeatMode mode) noexcept;
    void setShuffleEnabled(bool enabled) noexcept;

private:
    domain::Playlist& playlist_;
};

} // namespace player::playlist::application
