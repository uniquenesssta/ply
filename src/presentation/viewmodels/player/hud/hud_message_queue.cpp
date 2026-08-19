#include "presentation/viewmodels/player/hud/hud_message_queue.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace player::presentation {

HudMessageQueue::HudMessageQueue(QObject* parent)
    : QObject(parent)
{
    holdTimer_.setSingleShot(true);
    holdTimer_.setInterval(holdDurationMs());
    connect(&holdTimer_, &QTimer::timeout, this, &HudMessageQueue::advance);
}

bool HudMessageQueue::visible() const noexcept
{
    return current_.has_value();
}

QString HudMessageQueue::messageKey() const
{
    return current_.has_value() ? current_->key : QString{};
}

QString HudMessageQueue::valueText() const
{
    return current_.has_value() ? current_->value : QString{};
}

void HudMessageQueue::showVolume(double volumePercent, bool muted)
{
    if (!std::isfinite(volumePercent)) {
        return;
    }

    const double clamped = std::clamp(volumePercent, 0.0, 100.0);
    Message message;
    message.coalescingKey = CoalescingKey::Volume;
    message.key = muted ? QStringLiteral("muted") : QStringLiteral("volume");
    message.value = muted
        ? QString{}
        : QStringLiteral("%1%").arg(static_cast<qlonglong>(std::lround(clamped)));
    enqueueOrUpdate(std::move(message));
}

void HudMessageQueue::showSeek(const QString& positionText)
{
    const QString normalized = positionText.trimmed();
    if (normalized.isEmpty()) {
        return;
    }

    Message message;
    message.coalescingKey = CoalescingKey::Seek;
    message.key = QStringLiteral("seek");
    message.value = normalized;
    enqueueOrUpdate(std::move(message));
}

void HudMessageQueue::showSeekFailure()
{
    Message message;
    message.coalescingKey = CoalescingKey::Important;
    message.priority = Priority::Important;
    message.key = QStringLiteral("seekFailed");
    enqueueOrUpdate(std::move(message));
}

void HudMessageQueue::showSpeed(const QString& speedText)
{
    const QString normalized = speedText.trimmed();
    if (normalized.isEmpty()) {
        return;
    }

    Message message;
    message.coalescingKey = CoalescingKey::Speed;
    message.key = QStringLiteral("speed");
    message.value = normalized;
    enqueueOrUpdate(std::move(message));
}

void HudMessageQueue::showTrackChange(const QString& trackText)
{
    const QString normalized = trackText.trimmed();
    if (normalized.isEmpty()) {
        return;
    }

    Message message;
    message.coalescingKey = CoalescingKey::Track;
    message.key = QStringLiteral("track");
    message.value = normalized;
    enqueueOrUpdate(std::move(message));
}

void HudMessageQueue::showSubtitleDelay(int milliseconds)
{
    Message message;
    message.coalescingKey = CoalescingKey::SubtitleDelay;
    message.key = QStringLiteral("subtitleDelay");
    message.value = milliseconds > 0
        ? QStringLiteral("+%1 ms").arg(milliseconds)
        : QStringLiteral("%1 ms").arg(milliseconds);
    enqueueOrUpdate(std::move(message));
}

void HudMessageQueue::clear()
{
    holdTimer_.stop();
    pending_.clear();
    if (!current_.has_value()) {
        return;
    }

    current_.reset();
    emit stateChanged();
}

void HudMessageQueue::enqueueOrUpdate(Message message)
{
    if (!current_.has_value()) {
        present(std::move(message));
        return;
    }

    if (current_->coalescingKey == message.coalescingKey) {
        removePending(message.coalescingKey);
        current_ = std::move(message);
        restartTimer();
        emit stateChanged();
        return;
    }

    if (outranks(message.priority, current_->priority)) {
        removePending(message.coalescingKey);
        discardPendingBelow(message.priority);
        present(std::move(message));
        return;
    }

    const auto pendingIt = std::find_if(
        pending_.begin(),
        pending_.end(),
        [&message](const Message& pending) {
            return pending.coalescingKey == message.coalescingKey;
        });
    if (pendingIt != pending_.end()) {
        *pendingIt = std::move(message);
        return;
    }

    pushPending(std::move(message));
}

void HudMessageQueue::present(Message message)
{
    current_ = std::move(message);
    restartTimer();
    emit stateChanged();
}

void HudMessageQueue::advance()
{
    if (pending_.empty()) {
        if (current_.has_value()) {
            current_.reset();
            emit stateChanged();
        }
        return;
    }

    Message next = std::move(pending_.front());
    pending_.pop_front();
    present(std::move(next));
}

void HudMessageQueue::restartTimer()
{
    holdTimer_.start(holdDurationMs());
}

void HudMessageQueue::removePending(CoalescingKey coalescingKey)
{
    std::erase_if(
        pending_,
        [coalescingKey](const Message& message) {
            return message.coalescingKey == coalescingKey;
        });
}

void HudMessageQueue::discardPendingBelow(Priority priority)
{
    std::erase_if(
        pending_,
        [priority](const Message& message) {
            return outranks(priority, message.priority);
        });
}

void HudMessageQueue::pushPending(Message message)
{
    if (static_cast<qsizetype>(pending_.size()) < kMaximumPendingMessages) {
        pending_.push_back(std::move(message));
        return;
    }

    const auto lowerPriorityIt = std::find_if(
        pending_.begin(),
        pending_.end(),
        [&message](const Message& pending) {
            return outranks(message.priority, pending.priority);
        });
    if (lowerPriorityIt != pending_.end()) {
        pending_.erase(lowerPriorityIt);
        pending_.push_back(std::move(message));
        return;
    }

    if (message.priority == Priority::Transient) {
        const auto transientIt = std::find_if(
            pending_.begin(),
            pending_.end(),
            [](const Message& pending) {
                return pending.priority == Priority::Transient;
            });
        if (transientIt == pending_.end()) {
            return;
        }
        pending_.erase(transientIt);
        pending_.push_back(std::move(message));
        return;
    }

    pending_.pop_front();
    pending_.push_back(std::move(message));
}

bool HudMessageQueue::outranks(Priority candidate, Priority current) noexcept
{
    return static_cast<int>(candidate) > static_cast<int>(current);
}

} // namespace player::presentation
