#pragma once

#include <atomic>
#include <cstdint>

namespace player::playback::infrastructure::mpv::render {

class MpvVideoItem;

struct MpvRenderVisibilityInput final
{
    bool itemVisible = false;
    bool windowVisible = false;
    bool windowMinimized = false;
};

struct MpvRenderVisibilitySnapshot final
{
    bool updatesAllowed = false;
    std::uint64_t revision = 0;
};

class MpvRenderVisibilityPolicy final
{
public:
    MpvRenderVisibilityPolicy() = default;

    [[nodiscard]] MpvRenderVisibilitySnapshot snapshot() const noexcept;

private:
    friend class MpvVideoItem;

    void update(MpvRenderVisibilityInput input) noexcept;

    // Bit 0 stores render eligibility. Remaining bits store a monotonic revision.
    // Keeping both values in one atomic gives callback/render threads a coherent
    // snapshot without introducing a second visibility owner.
    std::atomic<std::uint64_t> encodedState_{0};
};

} // namespace player::playback::infrastructure::mpv::render
