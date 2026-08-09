#include "state_publisher.h"

#include <QMetaType>
#include <QTimer>

#include <algorithm>

namespace player::playback::application {
namespace {

using player::playback::domain::PlaybackFailure;
using player::playback::domain::PlaybackSnapshot;

bool failuresEqual(
    const std::optional<PlaybackFailure>& left,
    const std::optional<PlaybackFailure>& right) noexcept
{
    if (left.has_value() != right.has_value()) {
        return false;
    }
    if (!left.has_value()) {
        return true;
    }

    return left->category == right->category
        && left->backendCode == right->backendCode
        && left->diagnostic == right->diagnostic;
}

} // namespace

StatePublisher::StatePublisher(QObject* parent)
    : QObject(parent)
    , positionTimer_(new QTimer(this))
{
    qRegisterMetaType<PlaybackSnapshot>(
        "player::playback::domain::PlaybackSnapshot");

    positionTimer_->setSingleShot(true);
    positionTimer_->setTimerType(Qt::PreciseTimer);
    QObject::connect(
        positionTimer_,
        &QTimer::timeout,
        this,
        &StatePublisher::flushPendingPosition);
}

void StatePublisher::reset()
{
    positionTimer_->stop();
    publishClock_.invalidate();
    lastReceived_.reset();
    pendingPosition_.reset();
}

void StatePublisher::acceptSnapshot(const PlaybackSnapshot& snapshot)
{
    if (!lastReceived_.has_value()) {
        lastReceived_ = snapshot;
        publishImmediately(snapshot);
        return;
    }

    const PlaybackSnapshot previous = *lastReceived_;
    if (snapshotsEqual(previous, snapshot)) {
        return;
    }

    const bool positionOnly = equalExceptPosition(previous, snapshot)
        && previous.timeline().positionSeconds != snapshot.timeline().positionSeconds;
    lastReceived_ = snapshot;

    if (positionOnly) {
        schedulePositionPublish(snapshot);
        return;
    }

    positionTimer_->stop();
    pendingPosition_.reset();
    publishImmediately(snapshot);
}

void StatePublisher::flushPendingPosition()
{
    if (!pendingPosition_.has_value()) {
        return;
    }

    const PlaybackSnapshot snapshot = *pendingPosition_;
    pendingPosition_.reset();
    publishImmediately(snapshot);
}

bool StatePublisher::snapshotsEqual(
    const PlaybackSnapshot& left,
    const PlaybackSnapshot& right) noexcept
{
    return equalExceptPosition(left, right)
        && left.timeline().positionSeconds == right.timeline().positionSeconds;
}

bool StatePublisher::equalExceptPosition(
    const PlaybackSnapshot& left,
    const PlaybackSnapshot& right) noexcept
{
    return left.generation() == right.generation()
        && left.lifecycle() == right.lifecycle()
        && left.transport() == right.transport()
        && left.media().source == right.media().source
        && left.media().title == right.media().title
        && left.media().path == right.media().path
        && left.timeline().durationSeconds == right.timeline().durationSeconds
        && left.timeline().seekable == right.timeline().seekable
        && left.timeline().seeking == right.timeline().seeking
        && left.buffering().active == right.buffering().active
        && left.buffering().progressPercent == right.buffering().progressPercent
        && left.controls().volumePercent == right.controls().volumePercent
        && left.controls().muted == right.controls().muted
        && left.controls().speed == right.controls().speed
        && failuresEqual(left.failure(), right.failure());
}

void StatePublisher::publishImmediately(const PlaybackSnapshot& snapshot)
{
    emit snapshotPublished(snapshot);
    publishClock_.start();
}

void StatePublisher::schedulePositionPublish(const PlaybackSnapshot& snapshot)
{
    if (!publishClock_.isValid()
        || publishClock_.elapsed() >= positionPublishIntervalMilliseconds()) {
        positionTimer_->stop();
        pendingPosition_.reset();
        publishImmediately(snapshot);
        return;
    }

    pendingPosition_ = snapshot;
    if (positionTimer_->isActive()) {
        return;
    }

    const qint64 remaining = positionPublishIntervalMilliseconds() - publishClock_.elapsed();
    positionTimer_->start(static_cast<int>(std::max<qint64>(1, remaining)));
}

} // namespace player::playback::application
