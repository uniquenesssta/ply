#include "presentation/viewmodels/player/media/player_media_view_model.h"

namespace player::presentation {
namespace {

bool lifecycleHasEstablishedMedia(
    player::playback::domain::PlaybackLifecycleState lifecycle) noexcept
{
    using player::playback::domain::PlaybackLifecycleState;
    return lifecycle == PlaybackLifecycleState::Ready
        || lifecycle == PlaybackLifecycleState::Ended;
}

} // namespace

PlayerMediaViewModel::PlayerMediaViewModel(QObject* parent)
    : QObject(parent)
{
}

bool PlayerMediaViewModel::hasMedia() const noexcept
{
    return hasMedia_;
}

bool PlayerMediaViewModel::hasVideo() const noexcept
{
    return hasVideo_;
}

void PlayerMediaViewModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const auto& source = snapshot.media().source;
    const bool nextHasMedia = lifecycleHasEstablishedMedia(snapshot.lifecycle())
        && source.has_value()
        && !source->isEmpty();
    const bool nextHasVideo = nextHasMedia && snapshot.streams().video.has_value();

    if (nextHasMedia == hasMedia_ && nextHasVideo == hasVideo_) {
        return;
    }

    hasMedia_ = nextHasMedia;
    hasVideo_ = nextHasVideo;
    emit stateChanged();
}

} // namespace player::presentation
