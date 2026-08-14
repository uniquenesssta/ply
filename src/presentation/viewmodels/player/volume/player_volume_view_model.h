#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QObject>

#include <optional>

namespace player::presentation {

class PlayerVolumeViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canAdjustVolume READ canAdjustVolume NOTIFY stateChanged)
    Q_PROPERTY(bool canToggleMute READ canToggleMute NOTIFY stateChanged)
    Q_PROPERTY(double normalizedVolume READ normalizedVolume NOTIFY stateChanged)
    Q_PROPERTY(double volumePercent READ volumePercent NOTIFY stateChanged)
    Q_PROPERTY(bool muted READ muted NOTIFY stateChanged)
    Q_PROPERTY(bool volumePending READ volumePending NOTIFY stateChanged)
    Q_PROPERTY(bool mutePending READ mutePending NOTIFY stateChanged)

public:
    explicit PlayerVolumeViewModel(QObject* parent = nullptr);

    [[nodiscard]] bool canAdjustVolume() const noexcept;
    [[nodiscard]] bool canToggleMute() const noexcept;
    [[nodiscard]] double normalizedVolume() const noexcept;
    [[nodiscard]] double volumePercent() const noexcept;
    [[nodiscard]] bool muted() const noexcept;
    [[nodiscard]] bool volumePending() const noexcept;
    [[nodiscard]] bool mutePending() const noexcept;

    Q_INVOKABLE bool requestVolumeNormalized(double normalizedVolume);
    Q_INVOKABLE bool requestToggleMuted();

    [[nodiscard]] bool rejectPendingVolume();
    [[nodiscard]] bool rejectPendingMute();

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();
    void volumeRequested(double percent);
    void mutedRequested(bool muted);

private:
    [[nodiscard]] bool observableStateChanged(
        bool oldCanAdjustVolume,
        bool oldCanToggleMute,
        double oldNormalizedVolume,
        double oldVolumePercent,
        bool oldMuted,
        bool oldVolumePending,
        bool oldMutePending) const noexcept;

    std::optional<double> backendVolumePercent_;
    std::optional<bool> backendMuted_;
    std::optional<double> pendingVolumePercent_;
    std::optional<bool> pendingMuted_;
};

} // namespace player::presentation
