#include "tracks/application/track_selection_controller.h"

namespace player::tracks::application {

TrackSelectionController::TrackSelectionController(QObject* parent)
    : QObject(parent)
{
}

bool TrackSelectionController::canSelectAudio() const noexcept
{
    return canSelectAudio_;
}

bool TrackSelectionController::canSelectSubtitle() const noexcept
{
    return canSelectSubtitle_;
}

bool TrackSelectionController::canSelectVideo() const noexcept
{
    return canSelectVideo_;
}

bool TrackSelectionController::requestSelectAudioTrack(qint64 trackId)
{
    return submitSelection(player::playback::domain::TrackKind::Audio, trackId);
}

bool TrackSelectionController::requestSelectSubtitleTrack(qint64 trackId)
{
    return submitSelection(player::playback::domain::TrackKind::Subtitle, trackId);
}

bool TrackSelectionController::requestSelectVideoTrack(qint64 trackId)
{
    return submitSelection(player::playback::domain::TrackKind::Video, trackId);
}

void TrackSelectionController::acceptCapabilities(
    bool hasAudio,
    bool hasSubtitle,
    bool hasVideo)
{
    const bool changed = canSelectAudio_ != hasAudio
        || canSelectSubtitle_ != hasSubtitle
        || canSelectVideo_ != hasVideo;
    canSelectAudio_ = hasAudio;
    canSelectSubtitle_ = hasSubtitle;
    canSelectVideo_ = hasVideo;
    if (changed) {
        emit capabilityChanged();
    }
}

bool TrackSelectionController::submitSelection(
    player::playback::domain::TrackKind kind,
    qint64 trackId)
{
    const bool enabled = kind == player::playback::domain::TrackKind::Audio
        ? canSelectAudio_
        : kind == player::playback::domain::TrackKind::Subtitle
            ? canSelectSubtitle_
            : canSelectVideo_;
    if (!enabled) {
        return false;
    }

    emit trackSelectionRequested(
        static_cast<int>(kind),
        trackId);
    return true;
}

} // namespace player::tracks::application
