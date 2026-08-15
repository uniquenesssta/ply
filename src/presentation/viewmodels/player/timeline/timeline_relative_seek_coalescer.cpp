#include "presentation/viewmodels/player/timeline/timeline_relative_seek_coalescer.h"

#include <cmath>

namespace player::presentation {

TimelineRelativeSeekCoalescer::TimelineRelativeSeekCoalescer(QObject* parent)
    : QObject(parent)
{
    timer_.setSingleShot(true);
    timer_.setInterval(kCoalesceWindowMilliseconds);
    QObject::connect(
        &timer_,
        &QTimer::timeout,
        this,
        &TimelineRelativeSeekCoalescer::flushPending);
}

bool TimelineRelativeSeekCoalescer::enqueue(double deltaSeconds)
{
    if (!std::isfinite(deltaSeconds)
        || std::abs(deltaSeconds) <= kZeroTolerance) {
        return false;
    }

    pendingDeltaSeconds_ += deltaSeconds;
    if (std::abs(pendingDeltaSeconds_) <= kZeroTolerance) {
        pendingDeltaSeconds_ = 0.0;
        timer_.stop();
        return true;
    }

    if (!timer_.isActive()) {
        timer_.start();
    }
    return true;
}

bool TimelineRelativeSeekCoalescer::clear()
{
    const bool changed = timer_.isActive()
        || std::abs(pendingDeltaSeconds_) > kZeroTolerance;
    timer_.stop();
    pendingDeltaSeconds_ = 0.0;
    return changed;
}

bool TimelineRelativeSeekCoalescer::hasPending() const noexcept
{
    return std::abs(pendingDeltaSeconds_) > kZeroTolerance;
}

double TimelineRelativeSeekCoalescer::pendingDeltaSeconds() const noexcept
{
    return pendingDeltaSeconds_;
}

void TimelineRelativeSeekCoalescer::flushPending()
{
    if (!hasPending()) {
        pendingDeltaSeconds_ = 0.0;
        return;
    }

    const double deltaSeconds = pendingDeltaSeconds_;
    pendingDeltaSeconds_ = 0.0;
    emit flushRequested(deltaSeconds);
}

} // namespace player::presentation
