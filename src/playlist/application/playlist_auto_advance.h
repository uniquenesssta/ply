#pragma once

#include <QtGlobal>

#include <optional>

namespace player::playlist::application {

class PlaylistController;

}

namespace player::playlist::domain {

class Playlist;

}

namespace player::playlist::application {

class PlaylistAutoAdvance final
{
public:
    PlaylistAutoAdvance(
        domain::Playlist& playlist,
        PlaylistController& controller) noexcept;

    void acceptPlaybackState(quint64 mediaGeneration, bool naturallyEnded);
    void acceptPlaybackFailure(quint64 mediaGeneration);

private:
    enum class TerminalReason : quint8
    {
        NaturalEnd,
        Failure,
    };

    void acceptTerminalState(quint64 mediaGeneration, TerminalReason reason);

    domain::Playlist& playlist_;
    PlaylistController& controller_;
    std::optional<quint64> handledTerminalGeneration_;
};

} // namespace player::playlist::application
