#include "presentation/viewmodels/player/transport/player_transport_view_model.h"

#include "playback/domain/state/playback_selectors.h"

namespace player::presentation {

PlayerTransportViewModel::PlayerTransportViewModel(QObject* parent)
    : QObject(parent)
{
}

bool PlayerTransportViewModel::canPlay() const noexcept
{
    return canPlay_;
}

bool PlayerTransportViewModel::canPause() const noexcept
{
    return canPause_;
}

bool PlayerTransportViewModel::canStop() const noexcept
{
    return canStop_;
}

bool PlayerTransportViewModel::isPlaying() const noexcept
{
    return isPlaying_;
}

bool PlayerTransportViewModel::canPrevious() const noexcept
{
    return false;
}

bool PlayerTransportViewModel::canNext() const noexcept
{
    return false;
}

bool PlayerTransportViewModel::requestPlay()
{
    if (!canPlay_) {
        return false;
    }

    emit playRequested();
    return true;
}

bool PlayerTransportViewModel::requestPause()
{
    if (!canPause_) {
        return false;
    }

    emit pauseRequested();
    return true;
}

bool PlayerTransportViewModel::requestTogglePlayPause()
{
    if (canPause_) {
        emit pauseRequested();
        return true;
    }
    if (canPlay_) {
        emit playRequested();
        return true;
    }
    return false;
}

bool PlayerTransportViewModel::requestStop()
{
    if (!canStop_) {
        return false;
    }

    emit stopRequested();
    return true;
}

bool PlayerTransportViewModel::requestPrevious()
{
    return false;
}

bool PlayerTransportViewModel::requestNext()
{
    return false;
}

void PlayerTransportViewModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const bool nextCanPlay = player::playback::domain::selectors::canPlay(snapshot);
    const bool nextCanPause = player::playback::domain::selectors::canPause(snapshot);
    const bool nextCanStop = player::playback::domain::selectors::canStop(snapshot);
    const bool nextIsPlaying = player::playback::domain::selectors::isPlaying(snapshot);

    if (nextCanPlay == canPlay_
        && nextCanPause == canPause_
        && nextCanStop == canStop_
        && nextIsPlaying == isPlaying_) {
        return;
    }

    canPlay_ = nextCanPlay;
    canPause_ = nextCanPause;
    canStop_ = nextCanStop;
    isPlaying_ = nextIsPlaying;
    emit stateChanged();
}

} // namespace player::presentation
