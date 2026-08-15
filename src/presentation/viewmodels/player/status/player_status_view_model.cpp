#include "presentation/viewmodels/player/status/player_status_view_model.h"

#include <algorithm>
#include <cmath>

namespace player::presentation {
namespace {

int normalizedBufferingPercent(
    PlayerStatusKind status,
    const player::playback::domain::PlaybackSnapshot& snapshot) noexcept
{
    if (status != PlayerStatusKind::Buffering
        || !snapshot.buffering().progressPercent.has_value()
        || !std::isfinite(*snapshot.buffering().progressPercent)) {
        return -1;
    }

    return static_cast<int>(std::lround(std::clamp(
        *snapshot.buffering().progressPercent,
        0.0,
        100.0)));
}

} // namespace

PlayerStatusViewModel::PlayerStatusViewModel(QObject* parent)
    : QObject(parent)
    , status_(selectPlayerStatus(player::playback::domain::PlaybackSnapshot{}))
{
}

PlayerStatusKind PlayerStatusViewModel::status() const noexcept
{
    return status_;
}

QString PlayerStatusViewModel::statusKey() const
{
    switch (status_) {
    case PlayerStatusKind::Empty:
        return QStringLiteral("empty");
    case PlayerStatusKind::Loading:
        return QStringLiteral("loading");
    case PlayerStatusKind::Buffering:
        return QStringLiteral("buffering");
    case PlayerStatusKind::Ended:
        return QStringLiteral("ended");
    case PlayerStatusKind::Error:
        return QStringLiteral("error");
    case PlayerStatusKind::None:
        return {};
    }

    return {};
}

bool PlayerStatusViewModel::visible() const noexcept
{
    return status_ != PlayerStatusKind::None;
}

bool PlayerStatusViewModel::errorVisible() const noexcept
{
    return status_ == PlayerStatusKind::Error;
}

int PlayerStatusViewModel::bufferingPercent() const noexcept
{
    return bufferingPercent_;
}

void PlayerStatusViewModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const PlayerStatusKind nextStatus = selectPlayerStatus(snapshot);
    const int nextBufferingPercent = normalizedBufferingPercent(nextStatus, snapshot);

    if (nextStatus == status_ && nextBufferingPercent == bufferingPercent_) {
        return;
    }

    status_ = nextStatus;
    bufferingPercent_ = nextBufferingPercent;
    emit stateChanged();
}

} // namespace player::presentation
