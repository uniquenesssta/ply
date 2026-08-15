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
    message.kind = MessageKind::Volume;
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
    message.kind = MessageKind::Seek;
    message.key = QStringLiteral("seek");
    message.value = normalized;
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

    if (current_->kind == message.kind) {
        removePending(message.kind);
        current_ = std::move(message);
        restartTimer();
        emit stateChanged();
        return;
    }

    const auto pendingIt = std::find_if(
        pending_.begin(),
        pending_.end(),
        [&message](const Message& pending) {
            return pending.kind == message.kind;
        });
    if (pendingIt != pending_.end()) {
        *pendingIt = std::move(message);
        return;
    }

    if (static_cast<qsizetype>(pending_.size()) >= kMaximumPendingMessages) {
        pending_.pop_front();
    }
    pending_.push_back(std::move(message));
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

void HudMessageQueue::removePending(MessageKind kind)
{
    std::erase_if(
        pending_,
        [kind](const Message& message) {
            return message.kind == kind;
        });
}

} // namespace player::presentation
