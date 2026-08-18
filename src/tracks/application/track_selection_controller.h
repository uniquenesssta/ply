#pragma once

#include "playback/domain/commands/track_selection_command.h"

#include <QObject>

#include <functional>

namespace player::tracks::application {

class TrackSelectionController final : public QObject
{
    Q_OBJECT

public:
    using SubmitSelection = std::function<bool(
        const player::playback::domain::TrackSelectionCommand&)>;

    explicit TrackSelectionController(
        SubmitSelection submitSelection,
        QObject* parent = nullptr);

    Q_INVOKABLE bool selectAudioTrack(qint64 trackId);
    Q_INVOKABLE bool selectSubtitleTrack(qint64 trackId);
    Q_INVOKABLE bool disableSubtitles();

private:
    [[nodiscard]] bool submit(
        player::playback::domain::TrackSelectionCommand command) const;

    SubmitSelection submitSelection_;
};

} // namespace player::tracks::application
