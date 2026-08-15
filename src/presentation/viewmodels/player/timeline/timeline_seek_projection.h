#pragma once

#include "playback/domain/state/media_generation.h"

#include <optional>

namespace player::presentation {

class TimelineSeekProjection final
{
public:
    [[nodiscard]] bool isPending() const noexcept;
    [[nodiscard]] player::playback::domain::MediaGeneration generation() const noexcept;
    [[nodiscard]] std::optional<double> targetSeconds() const noexcept;

    [[nodiscard]] bool beginAbsolute(
        player::playback::domain::MediaGeneration generation,
        double targetSeconds,
        double durationSeconds) noexcept;
    [[nodiscard]] std::optional<double> nudgeRelative(
        player::playback::domain::MediaGeneration generation,
        double actualSeconds,
        double deltaSeconds,
        double durationSeconds) noexcept;
    [[nodiscard]] bool acknowledge(
        double actualSeconds,
        double toleranceSeconds) noexcept;
    [[nodiscard]] bool cancelIfGenerationChanged(
        player::playback::domain::MediaGeneration generation) noexcept;
    [[nodiscard]] bool clear() noexcept;

private:
    [[nodiscard]] static bool isFiniteNonNegative(double value) noexcept;
    [[nodiscard]] static double clampToDuration(
        double value,
        double durationSeconds) noexcept;

    player::playback::domain::MediaGeneration generation_;
    std::optional<double> targetSeconds_;
};

} // namespace player::presentation
