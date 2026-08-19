#pragma once

#include "playback/domain/commands/audio_delay_command.h"
#include "playback/domain/state/media_generation.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QObject>

#include <functional>
#include <optional>

namespace player::tracks::application {

class AudioDelayController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY stateChanged)
    Q_PROPERTY(bool pending READ pending NOTIFY stateChanged)
    Q_PROPERTY(double delaySeconds READ delaySeconds NOTIFY stateChanged)
    Q_PROPERTY(double pendingTargetSeconds READ pendingTargetSeconds NOTIFY stateChanged)
    Q_PROPERTY(int delayMilliseconds READ delayMilliseconds NOTIFY stateChanged)
    Q_PROPERTY(int pendingTargetMilliseconds READ pendingTargetMilliseconds NOTIFY stateChanged)
    Q_PROPERTY(double minimumSeconds READ minimumSeconds CONSTANT)
    Q_PROPERTY(double maximumSeconds READ maximumSeconds CONSTANT)
    Q_PROPERTY(double stepSeconds READ stepSeconds CONSTANT)

public:
    using SubmitDelay = std::function<bool(
        const player::playback::domain::SetAudioDelayCommand&)>;

    explicit AudioDelayController(
        SubmitDelay submitDelay,
        QObject* parent = nullptr);

    [[nodiscard]] bool available() const noexcept;
    [[nodiscard]] bool pending() const noexcept;
    [[nodiscard]] double delaySeconds() const noexcept;
    [[nodiscard]] double pendingTargetSeconds() const noexcept;
    [[nodiscard]] int delayMilliseconds() const noexcept;
    [[nodiscard]] int pendingTargetMilliseconds() const noexcept;
    [[nodiscard]] double minimumSeconds() const noexcept;
    [[nodiscard]] double maximumSeconds() const noexcept;
    [[nodiscard]] double stepSeconds() const noexcept;

    Q_INVOKABLE bool setDelaySeconds(double seconds);
    Q_INVOKABLE bool resetDelay();

    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);
    [[nodiscard]] bool rejectPendingDelay();

signals:
    void stateChanged();
    void delayConfirmed(int milliseconds);

private:
    [[nodiscard]] static double quantize(double seconds) noexcept;
    [[nodiscard]] static bool nearlyEqual(double lhs, double rhs) noexcept;
    [[nodiscard]] static int toMilliseconds(double seconds) noexcept;

    SubmitDelay submitDelay_;
    player::playback::domain::MediaGeneration generation_;
    bool available_ = false;
    double delaySeconds_ = 0.0;
    std::optional<double> pendingTargetSeconds_;
};

} // namespace player::tracks::application
