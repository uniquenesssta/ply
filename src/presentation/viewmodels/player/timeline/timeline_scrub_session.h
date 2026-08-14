#pragma once

#include "playback/domain/state/media_generation.h"

#include <QtGlobal>

#include <optional>

namespace player::presentation {

enum class TimelineScrubPhase : quint8
{
    Idle,
    Scrubbing,
    PendingCommit,
};

class TimelineScrubSession final
{
public:
    [[nodiscard]] TimelineScrubPhase phase() const noexcept;
    [[nodiscard]] bool isActive() const noexcept;
    [[nodiscard]] bool isScrubbing() const noexcept;
    [[nodiscard]] bool isPendingCommit() const noexcept;
    [[nodiscard]] double previewNormalized() const noexcept;
    [[nodiscard]] player::playback::domain::MediaGeneration generation() const noexcept;

    [[nodiscard]] bool begin(
        player::playback::domain::MediaGeneration generation,
        double normalized) noexcept;
    [[nodiscard]] bool update(double normalized) noexcept;
    [[nodiscard]] std::optional<double> commit(double normalized) noexcept;
    [[nodiscard]] bool cancel() noexcept;
    [[nodiscard]] bool cancelIfGenerationChanged(
        player::playback::domain::MediaGeneration generation) noexcept;
    [[nodiscard]] bool acknowledgePending() noexcept;

private:
    [[nodiscard]] static bool isValidNormalized(double normalized) noexcept;
    [[nodiscard]] static double clampNormalized(double normalized) noexcept;
    void reset() noexcept;

    TimelineScrubPhase phase_ = TimelineScrubPhase::Idle;
    player::playback::domain::MediaGeneration generation_;
    double previewNormalized_ = 0.0;
};

} // namespace player::presentation
