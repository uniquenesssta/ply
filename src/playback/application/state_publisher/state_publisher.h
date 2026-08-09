#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QElapsedTimer>
#include <QObject>

#include <optional>

class QTimer;

namespace player::playback::application {

class StatePublisher final : public QObject
{
    Q_OBJECT

public:
    explicit StatePublisher(QObject* parent = nullptr);

    StatePublisher(const StatePublisher&) = delete;
    StatePublisher& operator=(const StatePublisher&) = delete;

    void reset();

    [[nodiscard]] static constexpr int positionPublishIntervalMilliseconds() noexcept
    {
        return 50;
    }

signals:
    void snapshotPublished(const player::playback::domain::PlaybackSnapshot& snapshot);

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

private slots:
    void flushPendingPosition();

private:
    [[nodiscard]] static bool snapshotsEqual(
        const player::playback::domain::PlaybackSnapshot& left,
        const player::playback::domain::PlaybackSnapshot& right) noexcept;
    [[nodiscard]] static bool equalExceptPosition(
        const player::playback::domain::PlaybackSnapshot& left,
        const player::playback::domain::PlaybackSnapshot& right) noexcept;

    void publishImmediately(const player::playback::domain::PlaybackSnapshot& snapshot);
    void schedulePositionPublish(const player::playback::domain::PlaybackSnapshot& snapshot);

    QTimer* positionTimer_ = nullptr;
    QElapsedTimer publishClock_;
    std::optional<player::playback::domain::PlaybackSnapshot> lastReceived_;
    std::optional<player::playback::domain::PlaybackSnapshot> pendingPosition_;
};

} // namespace player::playback::application
