#pragma once

#include "playlist/domain/playlist_entry_id.h"

#include <QtGlobal>

namespace player::playlist::domain {

class Playlist;

enum class PlaylistNavigationAction : quint8
{
    None = 0,
    SelectEntry,
    ReloadCurrent,
};

struct PlaylistNavigationDecision final
{
    PlaylistNavigationAction action = PlaylistNavigationAction::None;
    PlaylistEntryId targetEntryId;
};

class PlaylistNavigation final
{
public:
    [[nodiscard]] static PlaylistNavigationDecision afterNaturalEnd(
        Playlist& playlist);
    [[nodiscard]] static PlaylistNavigationDecision afterPlaybackFailure(
        Playlist& playlist);
};

} // namespace player::playlist::domain
