#include "tracks/application/track_selection_controller.h"

#include <optional>
#include <utility>

namespace player::tracks::application {

using player::playback::domain::TrackSelectionCommand;
using player::playback::domain::TrackSelectionKind;

TrackSelectionController::TrackSelectionController(
    SubmitSelection submitSelection,
    QObject* parent)
    : QObject(parent)
    , submitSelection_(std::move(submitSelection))
{
}

bool TrackSelectionController::selectAudioTrack(qint64 trackId)
{
    if (trackId <= 0) {
        return false;
    }
    return submit(TrackSelectionCommand{
        TrackSelectionKind::Audio,
        trackId,
    });
}

bool TrackSelectionController::selectSubtitleTrack(qint64 trackId)
{
    if (trackId <= 0) {
        return false;
    }
    return submit(TrackSelectionCommand{
        TrackSelectionKind::Subtitle,
        trackId,
    });
}

bool TrackSelectionController::disableSubtitles()
{
    return submit(TrackSelectionCommand{
        TrackSelectionKind::Subtitle,
        std::nullopt,
    });
}

bool TrackSelectionController::submit(TrackSelectionCommand command) const
{
    return submitSelection_ && submitSelection_(command);
}

} // namespace player::tracks::application
