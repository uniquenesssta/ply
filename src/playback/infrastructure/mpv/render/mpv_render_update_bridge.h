#pragma once

#include <QObject>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>

class QString;

namespace player::playback::infrastructure::mpv::render {

class MpvRenderContext;

class MpvRenderUpdateBridge final : public QObject
{
    Q_OBJECT

public:
    explicit MpvRenderUpdateBridge(QObject* parent = nullptr);
    ~MpvRenderUpdateBridge() override;

    MpvRenderUpdateBridge(const MpvRenderUpdateBridge&) = delete;
    MpvRenderUpdateBridge& operator=(const MpvRenderUpdateBridge&) = delete;

    [[nodiscard]] bool activate(
        MpvRenderContext& renderContext,
        QString* errorMessage = nullptr);
    [[nodiscard]] bool deactivate(QString* errorMessage = nullptr);

    [[nodiscard]] bool isActive() const noexcept;

signals:
    void updateRequested();

private:
    static void onRenderUpdate(void* context) noexcept;
    void dispatchRenderUpdate() noexcept;
    void deliverUpdateRequest(std::uint64_t activationEpoch);
    void waitForCallbacksToDrain() noexcept;

    mutable std::mutex stateMutex_;
    std::mutex callbackMutex_;
    std::condition_variable callbackCondition_;
    std::atomic_size_t callbacksInFlight_{0};
    MpvRenderContext* renderContext_ = nullptr;
    std::uint64_t activationEpoch_ = 0;
    bool callbackInstalled_ = false;
    bool active_ = false;
};

} // namespace player::playback::infrastructure::mpv::render
