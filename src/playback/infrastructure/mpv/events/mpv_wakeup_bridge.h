#pragma once

#include <QObject>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>

class QString;
struct mpv_handle;

namespace player::playback::mpv {

class MpvWakeupBridge final : public QObject
{
    Q_OBJECT

public:
    explicit MpvWakeupBridge(QObject* parent = nullptr);
    ~MpvWakeupBridge() override;

    MpvWakeupBridge(const MpvWakeupBridge&) = delete;
    MpvWakeupBridge& operator=(const MpvWakeupBridge&) = delete;

    [[nodiscard]] bool activate(mpv_handle* handle, QString* errorMessage = nullptr);
    void deactivate() noexcept;

    [[nodiscard]] bool isActive() const noexcept;

signals:
    void wakeupRequested();

private:
    static void onWakeup(void* context) noexcept;
    void dispatchWakeup() noexcept;
    void waitForCallbacksToDrain() noexcept;

    mutable std::mutex stateMutex_;
    std::mutex callbackMutex_;
    std::condition_variable callbackCondition_;
    std::atomic_size_t callbacksInFlight_{0};
    mpv_handle* handle_ = nullptr;
    bool active_ = false;
};

} // namespace player::playback::mpv
