#include "playback/infrastructure/mpv/events/mpv_event_loop.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/events/mpv_event_decoder.h"
#include "playback/infrastructure/mpv/events/mpv_wakeup_bridge.h"

#include <mpv/client.h>

#include <QThread>
#include <QString>

namespace player::playback::mpv {

MpvEventLoop::MpvEventLoop(MpvHandle& handle, QObject* parent)
    : QObject(parent)
    , handle_(handle)
    , wakeupBridge_(new MpvWakeupBridge(this))
{
    connect(
        wakeupBridge_,
        &MpvWakeupBridge::wakeupRequested,
        this,
        &MpvEventLoop::drainPendingEvents,
        Qt::QueuedConnection);
}

MpvEventLoop::~MpvEventLoop()
{
    stop();
}

bool MpvEventLoop::start(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!isOnOwningThread()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("MpvEventLoop::start must run on the event loop's owning Qt thread.");
        }
        return false;
    }

    if (running_) {
        return true;
    }

    if (!handle_.isOpen() || !handle_.isInitialized() || handle_.nativeHandle() == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot start the mpv event loop before the mpv handle is initialized.");
        }
        return false;
    }

    running_ = true;
    if (!wakeupBridge_->activate(handle_.nativeHandle(), errorMessage)) {
        running_ = false;
        return false;
    }

    drainPendingEvents();
    return true;
}

void MpvEventLoop::stop() noexcept
{
    running_ = false;
    if (wakeupBridge_ != nullptr) {
        wakeupBridge_->deactivate();
    }
}

bool MpvEventLoop::isRunning() const noexcept
{
    return running_;
}

void MpvEventLoop::drainPendingEvents()
{
    if (!running_ || !isOnOwningThread() || !handle_.isOpen() || handle_.nativeHandle() == nullptr) {
        return;
    }

    while (running_) {
        mpv_event* event = mpv_wait_event(handle_.nativeHandle(), 0.0);
        if (event == nullptr || event->event_id == MPV_EVENT_NONE) {
            return;
        }

        emit eventDecoded(MpvEventDecoder::decode(*event));
    }
}

bool MpvEventLoop::isOnOwningThread() const noexcept
{
    return QThread::currentThread() == thread();
}

} // namespace player::playback::mpv
