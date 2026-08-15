#include "presentation/viewmodels/player/timeline/player_timeline_view_model.h"

#include "playback/domain/state/playback_selectors.h"
#include "presentation/viewmodels/player/timeline/timeline_relative_seek_coalescer.h"

#include <QChar>

#include <algorithm>
#include <cmath>

namespace player::presentation {
namespace {

constexpr double kSeekAcknowledgementToleranceSeconds = 0.75;
constexpr double kStateComparisonTolerance = 0.0000001;

bool nearlyEqual(double left, double right) noexcept
{
    return std::abs(left - right) <= kStateComparisonTolerance;
}

} // namespace

PlayerTimelineViewModel::PlayerTimelineViewModel(QObject* parent)
    : QObject(parent)
    , relativeSeekCoalescer_(new TimelineRelativeSeekCoalescer(this))
{
    QObject::connect(
        relativeSeekCoalescer_,
        &TimelineRelativeSeekCoalescer::flushRequested,
        this,
        &PlayerTimelineViewModel::flushRelativeSeek);
}

bool PlayerTimelineViewModel::canSeek() const noexcept
{
    return canSeek_;
}

bool PlayerTimelineViewModel::isScrubbing() const noexcept
{
    return scrubSession_.isScrubbing();
}

bool PlayerTimelineViewModel::seekPending() const noexcept
{
    return seekProjection_.isPending()
        || (relativeSeekCoalescer_ != nullptr && relativeSeekCoalescer_->hasPending());
}

bool PlayerTimelineViewModel::backendSeeking() const noexcept
{
    return backendSeeking_;
}

double PlayerTimelineViewModel::displayedNormalized() const noexcept
{
    if (!durationSeconds_.has_value() || *durationSeconds_ <= 0.0) {
        return 0.0;
    }

    if (scrubSession_.isScrubbing()) {
        return scrubSession_.previewNormalized();
    }
    if (const auto pendingTarget = seekProjection_.targetSeconds(); pendingTarget.has_value()) {
        return std::clamp(*pendingTarget / *durationSeconds_, 0.0, 1.0);
    }

    if (!actualPositionSeconds_.has_value()) {
        return 0.0;
    }

    return std::clamp(*actualPositionSeconds_ / *durationSeconds_, 0.0, 1.0);
}

double PlayerTimelineViewModel::durationSeconds() const noexcept
{
    return durationSeconds_.value_or(0.0);
}

QString PlayerTimelineViewModel::positionText() const
{
    if (!actualPositionSeconds_.has_value()
        && !durationSeconds_.has_value()
        && !scrubSession_.isActive()
        && !seekProjection_.isPending()) {
        return QStringLiteral("--:--:--");
    }

    return formatTimecode(displayedPositionSeconds());
}

QString PlayerTimelineViewModel::durationText() const
{
    if (!durationSeconds_.has_value()) {
        return QStringLiteral("--:--:--");
    }

    return formatTimecode(*durationSeconds_);
}

bool PlayerTimelineViewModel::beginScrub(double normalized)
{
    if (!canSeek_ || !generation_.isValid()) {
        return false;
    }

    if (!scrubSession_.begin(generation_, normalized)) {
        return false;
    }

    (void)seekProjection_.clear();
    if (relativeSeekCoalescer_ != nullptr) {
        (void)relativeSeekCoalescer_->clear();
    }
    emit stateChanged();
    return true;
}

bool PlayerTimelineViewModel::updateScrub(double normalized)
{
    if (!scrubSession_.update(normalized)) {
        return false;
    }

    emit stateChanged();
    return true;
}

bool PlayerTimelineViewModel::commitScrub(double normalized)
{
    if (!canSeek_ || !durationSeconds_.has_value()) {
        return false;
    }

    const std::optional<double> committed = scrubSession_.commit(normalized);
    if (!committed.has_value()) {
        return false;
    }

    const double targetSeconds = *committed * *durationSeconds_;
    if (!seekProjection_.beginAbsolute(
            generation_,
            targetSeconds,
            *durationSeconds_)) {
        emit stateChanged();
        return false;
    }

    emit stateChanged();
    emit seekRequested(targetSeconds);
    return true;
}

bool PlayerTimelineViewModel::cancelScrub()
{
    if (!scrubSession_.isScrubbing()) {
        return false;
    }

    (void)scrubSession_.cancel();
    emit stateChanged();
    return true;
}

bool PlayerTimelineViewModel::requestRelativeSeek(double deltaSeconds)
{
    if (!canSeek_
        || isScrubbing()
        || !generation_.isValid()
        || !durationSeconds_.has_value()
        || !std::isfinite(deltaSeconds)
        || std::abs(deltaSeconds) <= kStateComparisonTolerance
        || (!actualPositionSeconds_.has_value() && !seekProjection_.isPending())
        || relativeSeekCoalescer_ == nullptr) {
        return false;
    }

    const bool wasPending = seekPending();
    if (!relativeSeekCoalescer_->enqueue(deltaSeconds)) {
        return false;
    }

    if (wasPending != seekPending()) {
        emit stateChanged();
    }
    return true;
}

bool PlayerTimelineViewModel::rejectPendingSeek()
{
    bool changed = seekProjection_.clear();
    if (relativeSeekCoalescer_ != nullptr) {
        changed = relativeSeekCoalescer_->clear() || changed;
    }
    if (!changed) {
        return false;
    }

    emit stateChanged();
    return true;
}

void PlayerTimelineViewModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const bool previousCanSeek = canSeek();
    const bool previousScrubbing = isScrubbing();
    const bool previousPending = seekPending();
    const bool previousBackendSeeking = backendSeeking();
    const double previousDisplayed = displayedNormalized();
    const double previousDuration = durationSeconds();
    const QString previousPositionText = positionText();
    const QString previousDurationText = durationText();

    const auto nextGeneration = snapshot.generation();
    if (nextGeneration != generation_) {
        (void)scrubSession_.cancelIfGenerationChanged(nextGeneration);
        (void)seekProjection_.cancelIfGenerationChanged(nextGeneration);
        if (relativeSeekCoalescer_ != nullptr) {
            (void)relativeSeekCoalescer_->clear();
        }
    }
    generation_ = nextGeneration;

    actualPositionSeconds_ = sanitizePosition(snapshot.timeline().positionSeconds);
    durationSeconds_ = sanitizeDuration(snapshot.timeline().durationSeconds);
    backendSeeking_ = snapshot.timeline().seeking.value_or(false);

    canSeek_ = player::playback::domain::selectors::canSeek(snapshot)
        && generation_.isValid()
        && durationSeconds_.has_value();

    if (!canSeek_) {
        (void)scrubSession_.cancel();
        (void)seekProjection_.clear();
        if (relativeSeekCoalescer_ != nullptr) {
            (void)relativeSeekCoalescer_->clear();
        }
    } else if (actualPositionSeconds_.has_value()) {
        (void)seekProjection_.acknowledge(
            *actualPositionSeconds_,
            kSeekAcknowledgementToleranceSeconds);
    }

    const bool changed = previousCanSeek != canSeek()
        || previousScrubbing != isScrubbing()
        || previousPending != seekPending()
        || previousBackendSeeking != backendSeeking()
        || !nearlyEqual(previousDisplayed, displayedNormalized())
        || !nearlyEqual(previousDuration, durationSeconds())
        || previousPositionText != positionText()
        || previousDurationText != durationText();

    if (changed) {
        emit stateChanged();
    }
}

void PlayerTimelineViewModel::flushRelativeSeek(double deltaSeconds)
{
    if (!canSeek_
        || isScrubbing()
        || !generation_.isValid()
        || !durationSeconds_.has_value()) {
        emit stateChanged();
        return;
    }

    const std::optional<double> projectedTarget = seekProjection_.targetSeconds();
    if (!actualPositionSeconds_.has_value() && !projectedTarget.has_value()) {
        emit stateChanged();
        return;
    }

    const double baseActualSeconds = actualPositionSeconds_.value_or(*projectedTarget);
    const std::optional<double> effectiveDelta = seekProjection_.nudgeRelative(
        generation_,
        baseActualSeconds,
        deltaSeconds,
        *durationSeconds_);
    if (!effectiveDelta.has_value()
        || std::abs(*effectiveDelta) <= kStateComparisonTolerance) {
        emit stateChanged();
        return;
    }

    emit stateChanged();
    emit relativeSeekRequested(*effectiveDelta);
}

double PlayerTimelineViewModel::displayedPositionSeconds() const noexcept
{
    if (durationSeconds_.has_value() && scrubSession_.isScrubbing()) {
        return scrubSession_.previewNormalized() * *durationSeconds_;
    }
    if (const auto pendingTarget = seekProjection_.targetSeconds(); pendingTarget.has_value()) {
        if (!durationSeconds_.has_value()) {
            return std::max(0.0, *pendingTarget);
        }
        return std::clamp(*pendingTarget, 0.0, *durationSeconds_);
    }

    const double position = actualPositionSeconds_.value_or(0.0);
    if (!durationSeconds_.has_value()) {
        return std::max(0.0, position);
    }

    return std::clamp(position, 0.0, *durationSeconds_);
}

QString PlayerTimelineViewModel::formatTimecode(double seconds)
{
    if (!std::isfinite(seconds) || seconds < 0.0) {
        return QStringLiteral("--:--:--");
    }

    const qint64 totalSeconds = static_cast<qint64>(std::floor(seconds));
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 remainingSeconds = totalSeconds % 60;

    return QStringLiteral("%1:%2:%3")
        .arg(hours, 2, 10, QChar(u'0'))
        .arg(minutes, 2, 10, QChar(u'0'))
        .arg(remainingSeconds, 2, 10, QChar(u'0'));
}

std::optional<double> PlayerTimelineViewModel::sanitizePosition(
    const std::optional<double>& seconds) noexcept
{
    if (!seconds.has_value() || !std::isfinite(*seconds)) {
        return std::nullopt;
    }
    return std::max(0.0, *seconds);
}

std::optional<double> PlayerTimelineViewModel::sanitizeDuration(
    const std::optional<double>& seconds) noexcept
{
    if (!seconds.has_value() || !std::isfinite(*seconds) || *seconds <= 0.0) {
        return std::nullopt;
    }
    return *seconds;
}

} // namespace player::presentation
