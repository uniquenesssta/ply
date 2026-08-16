#include "request_timeout_monitor.h"

#include "request_tracker.h"

#include <QTimer>

#include <chrono>
#include <utility>

namespace player::playback::application {
namespace {

constexpr std::chrono::milliseconds kRequestTimeout{30000};
constexpr int kRequestTimeoutPollMilliseconds = 1000;

} // namespace

RequestTimeoutMonitor::RequestTimeoutMonitor(
    RequestTracker& tracker,
    QObject* parent)
    : RequestTimeoutMonitor(tracker, TimeoutHandler{}, parent)
{
}

RequestTimeoutMonitor::RequestTimeoutMonitor(
    RequestTracker& tracker,
    TimeoutHandler timeoutHandler,
    QObject* parent)
    : QObject(parent)
    , tracker_(tracker)
    , timeoutHandler_(std::move(timeoutHandler))
    , timer_(new QTimer(this))
{
    timer_->setInterval(kRequestTimeoutPollMilliseconds);
    timer_->setTimerType(Qt::CoarseTimer);
    QObject::connect(
        timer_,
        &QTimer::timeout,
        this,
        [this]() {
            if (!timeoutHandler_) {
                (void)tracker_.cancelExpired(
                    PlaybackRequestClock::now(),
                    kRequestTimeout);
                return;
            }

            const auto expired = tracker_.cancelExpiredRecords(
                PlaybackRequestClock::now(),
                kRequestTimeout);
            for (const PlaybackRequestRecord& record : expired) {
                timeoutHandler_(record);
            }
        });
}

void RequestTimeoutMonitor::start()
{
    timer_->start();
}

void RequestTimeoutMonitor::stop()
{
    timer_->stop();
}

} // namespace player::playback::application
