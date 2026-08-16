#pragma once

#include "media/domain/media_source.h"
#include "playlist/domain/playlist_entry_id.h"

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

    [[nodiscard]] bool remove(domain::PlaylistEntryId id);
    [[nodiscard]] bool move(domain::PlaylistEntryId id, std::size_t targetIndex);

private:
    domain::Playlist& playlist_;
};

} // namespace player::playlist::application
