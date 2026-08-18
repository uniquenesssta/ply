#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QObject>
#include <QtGlobal>

#include <optional>

namespace player::tracks::application {

// Audio delay intent controller (R8-06). Independent from subtitle delay:
// the two axes never share state or commands. Positive seconds shift audio
// later relative to video.
class AudioDelayController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canAdjust READ canAdjust NOTIFY stateChanged)
    Q_PROPERTY(double delaySeconds READ delaySeconds NOTIFY stateChanged)
    Q_PROPERTY(QString delayText READ delayText NOTIFY stateChanged)
    Q_PROPERTY(double stepSeconds READ stepSeconds CONSTANT)
    Q_PROPERTY(double minimumSeconds READ minimumSeconds CONSTANT)
    Q_PROPERTY(double maximumSeconds READ maximumSeconds CONSTANT)

public:
    explicit AudioDelayController(QObject* parent = nullptr);

    [[nodiscard]] bool canAdjust() const noexcept;
    [[nodiscard]] double delaySeconds() const noexcept;
    [[nodiscard]] QString delayText() const;
    [[nodiscard]] double stepSeconds() const noexcept;
    [[nodiscard]] double minimumSeconds() const noexcept;
    [[nodiscard]] double maximumSeconds() const noexcept;

    Q_INVOKABLE bool nudgeEarlier();
    Q_INVOKABLE bool nudgeLater();
    Q_INVOKABLE bool reset();
    Q_INVOKABLE bool requestSet(double seconds);

    [[nodiscard]] bool rejectPending();

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();
    void audioDelayRequested(double seconds);

private:
    [[nodiscard]] bool submit(double seconds);

    std::optional<double> backendDelaySeconds_;
    std::optional<double> pendingDelaySeconds_;
};

} // namespace player::tracks::application
