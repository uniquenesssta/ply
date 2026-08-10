#include "mpv_render_visibility_policy.h"

namespace player::playback::infrastructure::mpv::render {
namespace {

constexpr std::uint64_t kAllowedMask = 1U;
constexpr unsigned int kRevisionShift = 1U;

bool updatesAllowed(MpvRenderVisibilityInput input) noexcept
{
    return input.itemVisible && input.windowVisible && !input.windowMinimized;
}

} // namespace

MpvRenderVisibilitySnapshot MpvRenderVisibilityPolicy::snapshot() const noexcept
{
    const std::uint64_t encoded = encodedState_.load(std::memory_order_acquire);
    return MpvRenderVisibilitySnapshot{
        (encoded & kAllowedMask) != 0U,
        encoded >> kRevisionShift,
    };
}

void MpvRenderVisibilityPolicy::update(MpvRenderVisibilityInput input) noexcept
{
    const bool nextAllowed = updatesAllowed(input);
    std::uint64_t current = encodedState_.load(std::memory_order_acquire);

    while (((current & kAllowedMask) != 0U) != nextAllowed) {
        const std::uint64_t nextRevision = (current >> kRevisionShift) + 1U;
        const std::uint64_t next = (nextRevision << kRevisionShift)
            | (nextAllowed ? kAllowedMask : 0U);

        if (encodedState_.compare_exchange_weak(
                current,
                next,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
            return;
        }
    }
}

} // namespace player::playback::infrastructure::mpv::render
