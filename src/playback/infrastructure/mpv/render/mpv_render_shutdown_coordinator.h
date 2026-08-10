#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>

namespace player::playback::infrastructure::mpv::render {

class MpvVideoRenderer;

struct MpvRenderShutdownSnapshot final
{
    bool shutdownRequested = false;
    std::size_t activeRenderSections = 0;
    std::size_t liveRenderContexts = 0;
    bool renderReleased = false;
};

class MpvRenderShutdownCoordinator final
{
public:
    class RenderSection final
    {
    public:
        RenderSection() noexcept = default;
        ~RenderSection();

        RenderSection(const RenderSection&) = delete;
        RenderSection& operator=(const RenderSection&) = delete;
        RenderSection(RenderSection&& other) noexcept;
        RenderSection& operator=(RenderSection&& other) noexcept;

        [[nodiscard]] explicit operator bool() const noexcept;

    private:
        friend class MpvRenderShutdownCoordinator;

        explicit RenderSection(MpvRenderShutdownCoordinator* coordinator) noexcept;
        void reset() noexcept;

        MpvRenderShutdownCoordinator* coordinator_ = nullptr;
    };

    [[nodiscard]] bool beginShutdown() noexcept;
    [[nodiscard]] bool isShutdownRequested() const noexcept;
    [[nodiscard]] RenderSection tryEnterRenderSection() noexcept;
    [[nodiscard]] bool waitForRenderRelease(std::chrono::milliseconds timeout) const noexcept;
    [[nodiscard]] MpvRenderShutdownSnapshot snapshot() const noexcept;

private:
    friend class MpvVideoRenderer;

    void noteRenderContextCreated() noexcept;
    void noteRenderContextReleased() noexcept;
    void leaveRenderSection() noexcept;

    std::atomic_bool shutdownRequested_{false};
    mutable std::mutex stateMutex_;
    mutable std::condition_variable stateCondition_;
    std::size_t activeRenderSections_ = 0;
    std::size_t liveRenderContexts_ = 0;
};

} // namespace player::playback::infrastructure::mpv::render
