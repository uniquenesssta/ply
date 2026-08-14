#include "presentation/viewmodels/player/timeline/player_timeline_view_model.h"

#include "playback/domain/state/playback_selectors.h"

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
{
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
    return scrubSession_.isPendingCommit();
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
    if (scrubSession_.isPendingCommit() && pendingAbsoluteSeconds_.has_value()) {
        return std::clamp(*pendingAbsoluteSeconds_ / *durationSeconds_, 0.0, 1.0);
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
        && !scrubSession_.isActive()) {
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

    pendingAbsoluteSeconds_.reset();
    pendingSawBackendSeeking_ = false;
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

    pendingAbsoluteSeconds_ = *committed * *durationSeconds_;
    pendingSawBackendSeeking_ = backendSeeking_;
    emit stateChanged();
    emit seekRequested(*pendingAbsoluteSeconds_);
    return true;
}

bool PlayerTimelineViewModel::cancelScrub()
{
    if (!scrubSession_.isScrubbing()) {
        return false;
    }

    (void)scrubSession_.cancel();
    pendingAbsoluteSeconds_.reset();
    pendingSawBackendSeeking_ = false;
    emit stateChanged();
    return true;
}

bool PlayerTimelineViewModel::rejectPendingSeek()
{
    if (!scrubSession_.acknowledgePending()) {
        return false;
    }

    pendingAbsoluteSeconds_.reset();
    pendingSawBackendSeeking_ = false;
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
        pendingAbsoluteSeconds_.reset();
        pendingSawBackendSeeking_ = false;
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
        pendingAbsoluteSeconds_.reset();
        pendingSawBackendSeeking_ = false;
    } else if (scrubSession_.isPendingCommit()) {
        if (backendSeeking_) {
            pendingSawBackendSeeking_ = true;
        }

        const double targetSeconds = pendingAbsoluteSeconds_.value_or(
            scrubSession_.previewNormalized() * *durationSeconds_);
        const bool targetObserved = actualPositionSeconds_.has_value()
            && std::abs(*actualPositionSeconds_ - targetSeconds)
                <= kSeekAcknowledgementToleranceSeconds;
        const bool seekCycleCompleted =
            pendingSawBackendSeeking_ && !backendSeeking_;

        if (targetObserved || seekCycleCompleted) {
            (void)scrubSession_.acknowledgePending();
            pendingAbsoluteSeconds_.reset();
            pendingSawBackendSeeking_ = false;
        }
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

double PlayerTimelineViewModel::displayedPositionSeconds() const noexcept
{
    if (durationSeconds_.has_value() && scrubSession_.isScrubbing()) {
        return scrubSession_.previewNormalized() * *durationSeconds_;
    }
    if (scrubSession_.isPendingCommit() && pendingAbsoluteSeconds_.has_value()) {
        if (!durationSeconds_.has_value()) {
            return std::max(0.0, *pendingAbsoluteSeconds_);
        }
        return std::clamp(*pendingAbsoluteSeconds_, 0.0, *durationSeconds_);
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
