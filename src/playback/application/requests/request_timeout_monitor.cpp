#include "request_timeout_monitor.h"

#include "request_tracker.h"

#include <QTimer>

#include <chrono>

namespace player::playback::application {
namespace {

constexpr std::chrono::milliseconds kRequestTimeout{30000};
constexpr int kRequestTimeoutPollMilliseconds = 1000;

} // namespace

RequestTimeoutMonitor::RequestTimeoutMonitor(RequestTracker& tracker, QObject* parent)
    : QObject(parent)
    , tracker_(tracker)
    , timer_(new QTimer(this))
{
    timer_->setInterval(kRequestTimeoutPollMilliseconds);
    timer_->setTimerType(Qt::CoarseTimer);
    QObject::connect(
        timer_,
        &QTimer::timeout,
        this,
        [this]() {
            (void)tracker_.cancelExpired(
                PlaybackRequestClock::now(),
                kRequestTimeout);
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
