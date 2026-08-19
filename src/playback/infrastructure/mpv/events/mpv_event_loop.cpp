#include "playback/infrastructure/mpv/events/mpv_event_loop.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/events/mpv_event_decoder.h"
#include "playback/infrastructure/mpv/events/mpv_wakeup_bridge.h"

#include <mpv/client.h>

#include <QLoggingCategory>
#include <QString>
#include <QThread>

#include <variant>

namespace player::playback::mpv {
namespace {

Q_LOGGING_CATEGORY(mpvRuntimeDiagnostics, "playback.mpv.runtime")

bool isSubtitleFontDiagnosticPrefix(const QString& prefix)
{
    return prefix.contains(QStringLiteral("ass"), Qt::CaseInsensitive)
        || prefix.contains(QStringLiteral("sub"), Qt::CaseInsensitive)
        || prefix.contains(QStringLiteral("mkv"), Qt::CaseInsensitive)
        || prefix.contains(QStringLiteral("font"), Qt::CaseInsensitive);
}

bool isWarningOrErrorLevel(const QString& level)
{
    return level.compare(QStringLiteral("fatal"), Qt::CaseInsensitive) == 0
        || level.compare(QStringLiteral("error"), Qt::CaseInsensitive) == 0
        || level.compare(QStringLiteral("warn"), Qt::CaseInsensitive) == 0;
}

void forwardRuntimeDiagnostic(const MpvEvent& event)
{
    const auto* message = std::get_if<MpvLogMessageData>(&event.payload);
    if (event.type != MpvEventType::LogMessage || message == nullptr) {
        return;
    }

    const QString text = message->text.trimmed();
    if (text.isEmpty()) {
        return;
    }

    if (!isWarningOrErrorLevel(message->level)
        && !isSubtitleFontDiagnosticPrefix(message->prefix)) {
        return;
    }

    const QString diagnostic = QStringLiteral("mpv[%1] %2: %3")
                                   .arg(message->prefix, message->level, text);

    if (message->level.compare(QStringLiteral("fatal"), Qt::CaseInsensitive) == 0
        || message->level.compare(QStringLiteral("error"), Qt::CaseInsensitive) == 0) {
        qCCritical(mpvRuntimeDiagnostics).noquote() << diagnostic;
        return;
    }

    if (message->level.compare(QStringLiteral("warn"), Qt::CaseInsensitive) == 0) {
        qCWarning(mpvRuntimeDiagnostics).noquote() << diagnostic;
        return;
    }

    qCInfo(mpvRuntimeDiagnostics).noquote() << diagnostic;
}

} // namespace

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

    const int logRequestResult = mpv_request_log_messages(handle_.nativeHandle(), "debug");
    if (logRequestResult < 0) {
        qCWarning(mpvRuntimeDiagnostics).noquote()
            << QStringLiteral("Unable to request mpv diagnostic log messages: %1 (%2).")
                   .arg(QString::fromUtf8(mpv_error_string(logRequestResult)))
                   .arg(logRequestResult);
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

        const MpvEvent decoded = MpvEventDecoder::decode(*event);
        forwardRuntimeDiagnostic(decoded);
        emit eventDecoded(decoded);
    }
}

bool MpvEventLoop::isOnOwningThread() const noexcept
{
    return QThread::currentThread() == thread();
}

} // namespace player::playback::mpv
