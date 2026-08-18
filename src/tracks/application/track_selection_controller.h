#pragma once

#include "playback/domain/models/track_descriptor.h"

#include <QObject>
#include <QtGlobal>

namespace player::tracks::application {

// Accepts track-selection intents and forwards them as PlaybackCommands.
// Selected truth always comes from PlaybackSnapshot; this controller only
// carries intent and never invents selection state.
class TrackSelectionController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSelectAudio READ canSelectAudio NOTIFY capabilityChanged)
    Q_PROPERTY(bool canSelectSubtitle READ canSelectSubtitle NOTIFY capabilityChanged)
    Q_PROPERTY(bool canSelectVideo READ canSelectVideo NOTIFY capabilityChanged)

public:
    explicit TrackSelectionController(QObject* parent = nullptr);

    [[nodiscard]] bool canSelectAudio() const noexcept;
    [[nodiscard]] bool canSelectSubtitle() const noexcept;
    [[nodiscard]] bool canSelectVideo() const noexcept;

    // trackId <= 0 disables the family ('no').
    Q_INVOKABLE bool requestSelectAudioTrack(qint64 trackId);
    Q_INVOKABLE bool requestSelectSubtitleTrack(qint64 trackId);
    Q_INVOKABLE bool requestSelectVideoTrack(qint64 trackId);

public slots:
    void acceptCapabilities(
        bool hasAudio,
        bool hasSubtitle,
        bool hasVideo);

signals:
    void capabilityChanged();
    void trackSelectionRequested(int kind, qint64 trackId);

private:
    [[nodiscard]] bool submitSelection(
        player::playback::domain::TrackKind kind,
        qint64 trackId);

    bool canSelectAudio_ = false;
    bool canSelectSubtitle_ = false;
    bool canSelectVideo_ = false;
};

} // namespace player::tracks::application
