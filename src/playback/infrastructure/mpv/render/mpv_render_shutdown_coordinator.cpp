#include "mpv_render_shutdown_coordinator.h"

#include <utility>

namespace player::playback::infrastructure::mpv::render {

MpvRenderShutdownCoordinator::RenderSection::RenderSection(
    MpvRenderShutdownCoordinator* coordinator) noexcept
    : coordinator_(coordinator)
{
}

MpvRenderShutdownCoordinator::RenderSection::~RenderSection()
{
    reset();
}

MpvRenderShutdownCoordinator::RenderSection::RenderSection(RenderSection&& other) noexcept
    : coordinator_(std::exchange(other.coordinator_, nullptr))
{
}

MpvRenderShutdownCoordinator::RenderSection&
MpvRenderShutdownCoordinator::RenderSection::operator=(RenderSection&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    reset();
    coordinator_ = std::exchange(other.coordinator_, nullptr);
    return *this;
}

MpvRenderShutdownCoordinator::RenderSection::operator bool() const noexcept
{
    return coordinator_ != nullptr;
}

void MpvRenderShutdownCoordinator::RenderSection::reset() noexcept
{
    MpvRenderShutdownCoordinator* coordinator = std::exchange(coordinator_, nullptr);
    if (coordinator != nullptr) {
        coordinator->leaveRenderSection();
    }
}

bool MpvRenderShutdownCoordinator::beginShutdown() noexcept
{
    const bool wasRequested = shutdownRequested_.exchange(true, std::memory_order_acq_rel);
    stateCondition_.notify_all();
    return !wasRequested;
}

bool MpvRenderShutdownCoordinator::isShutdownRequested() const noexcept
{
    return shutdownRequested_.load(std::memory_order_acquire);
}

MpvRenderShutdownCoordinator::RenderSection
MpvRenderShutdownCoordinator::tryEnterRenderSection() noexcept
{
    std::scoped_lock lock(stateMutex_);
    if (shutdownRequested_.load(std::memory_order_acquire)) {
        return {};
    }

    ++activeRenderSections_;
    return RenderSection(this);
}

bool MpvRenderShutdownCoordinator::waitForRenderRelease(
    std::chrono::milliseconds timeout) noexcept
{
    std::unique_lock lock(stateMutex_);
    return stateCondition_.wait_for(lock, timeout, [this] {
        return shutdownRequested_.load(std::memory_order_acquire)
            && activeRenderSections_ == 0
            && liveRenderContexts_ == 0;
    });
}

MpvRenderShutdownSnapshot MpvRenderShutdownCoordinator::snapshot() const noexcept
{
    std::scoped_lock lock(stateMutex_);
    const bool shutdownRequested = shutdownRequested_.load(std::memory_order_acquire);
    return MpvRenderShutdownSnapshot{
        shutdownRequested,
        activeRenderSections_,
        liveRenderContexts_,
        shutdownRequested && activeRenderSections_ == 0 && liveRenderContexts_ == 0,
    };
}

void MpvRenderShutdownCoordinator::noteRenderContextCreated() noexcept
{
    std::scoped_lock lock(stateMutex_);
    ++liveRenderContexts_;
    stateCondition_.notify_all();
}

void MpvRenderShutdownCoordinator::noteRenderContextReleased() noexcept
{
    std::scoped_lock lock(stateMutex_);
    if (liveRenderContexts_ > 0) {
        --liveRenderContexts_;
    }
    stateCondition_.notify_all();
}

void MpvRenderShutdownCoordinator::leaveRenderSection() noexcept
{
    std::scoped_lock lock(stateMutex_);
    if (activeRenderSections_ > 0) {
        --activeRenderSections_;
    }
    stateCondition_.notify_all();
}

} // namespace player::playback::infrastructure::mpv::render
