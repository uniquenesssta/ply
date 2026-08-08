#include "playback/infrastructure/mpv/events/mpv_wakeup_bridge.h"

#include <mpv/client.h>

#include <QString>

#include <utility>

namespace player::playback::mpv {

MpvWakeupBridge::MpvWakeupBridge(QObject* parent)
    : QObject(parent)
{
}

MpvWakeupBridge::~MpvWakeupBridge()
{
    deactivate();
}

bool MpvWakeupBridge::activate(mpv_handle* handle, QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (handle == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot install an mpv wakeup callback on a null handle.");
        }
        return false;
    }

    {
        std::scoped_lock lock(stateMutex_);
        if (active_) {
            if (handle_ == handle) {
                return true;
            }

            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("MpvWakeupBridge is already active for another mpv handle.");
            }
            return false;
        }

        handle_ = handle;
        active_ = true;
    }

    mpv_set_wakeup_callback(handle, &MpvWakeupBridge::onWakeup, this);
    return true;
}

void MpvWakeupBridge::deactivate() noexcept
{
    mpv_handle* handle = nullptr;
    {
        std::scoped_lock lock(stateMutex_);
        active_ = false;
        handle = std::exchange(handle_, nullptr);
    }

    if (handle != nullptr) {
        mpv_set_wakeup_callback(handle, nullptr, nullptr);
    }

    waitForCallbacksToDrain();
}

bool MpvWakeupBridge::isActive() const noexcept
{
    std::scoped_lock lock(stateMutex_);
    return active_;
}

void MpvWakeupBridge::onWakeup(void* context) noexcept
{
    if (context == nullptr) {
        return;
    }

    static_cast<MpvWakeupBridge*>(context)->dispatchWakeup();
}

void MpvWakeupBridge::dispatchWakeup() noexcept
{
    callbacksInFlight_.fetch_add(1, std::memory_order_acq_rel);

    bool shouldWake = false;
    {
        std::scoped_lock lock(stateMutex_);
        shouldWake = active_;
    }

    if (shouldWake) {
        emit wakeupRequested();
    }

    if (callbacksInFlight_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        std::scoped_lock lock(callbackMutex_);
        callbackCondition_.notify_all();
    }
}

void MpvWakeupBridge::waitForCallbacksToDrain() noexcept
{
    std::unique_lock lock(callbackMutex_);
    callbackCondition_.wait(lock, [this] {
        return callbacksInFlight_.load(std::memory_order_acquire) == 0;
    });
}

} // namespace player::playback::mpv
