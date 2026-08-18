#pragma once

#include "playlist/domain/playlist_entry_id.h"

#include <QtGlobal>

namespace player::playlist::domain {

class Playlist;
class PlaylistSnapshot;

enum class PlaylistNavigationAction : quint8
{
    None = 0,
    SelectEntry,
    ReloadCurrent,
    StopPlayback,
};

struct PlaylistNavigationDecision final
{
    PlaylistNavigationAction action = PlaylistNavigationAction::None;
    PlaylistEntryId targetEntryId;
};

struct PlaylistNavigationCapabilities final
{
    bool canNext = false;
    bool canPrevious = false;
};

class PlaylistNavigation final
{
public:
    [[nodiscard]] static PlaylistNavigationCapabilities capabilities(
        const PlaylistSnapshot& snapshot) noexcept;
    [[nodiscard]] static PlaylistNavigationDecision afterNaturalEnd(
        Playlist& playlist);
    [[nodiscard]] static PlaylistNavigationDecision afterPlaybackFailure(
        Playlist& playlist);
    [[nodiscard]] static PlaylistNavigationDecision forCurrentRemoval(
        const Playlist& playlist,
        PlaylistEntryId currentId);
};

} // namespace player::playlist::domain
