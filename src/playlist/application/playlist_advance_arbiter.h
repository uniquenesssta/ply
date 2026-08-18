#pragma once

#include <QtGlobal>

#include <optional>

namespace player::playlist::application {

class PlaylistAdvanceArbiter final
{
public:
    void observeGeneration(quint64 mediaGeneration) noexcept;

    [[nodiscard]] bool tryClaimTerminal(quint64 mediaGeneration) noexcept;
    [[nodiscard]] bool tryClaimManualNavigation() noexcept;
    void releaseManualNavigationClaim() noexcept;

    void suppressObservedGeneration() noexcept;

private:
    [[nodiscard]] bool acceptGeneration(quint64 mediaGeneration) noexcept;

    std::optional<quint64> observedGeneration_;
    std::optional<quint64> claimedGeneration_;
    std::optional<quint64> suppressedGeneration_;
    bool manualClaimActive_ = false;
};

} // namespace player::playlist::application
