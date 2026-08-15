#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include <deque>
#include <optional>

namespace player::presentation {

class HudMessageQueue final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible NOTIFY stateChanged)
    Q_PROPERTY(QString messageKey READ messageKey NOTIFY stateChanged)
    Q_PROPERTY(QString valueText READ valueText NOTIFY stateChanged)

public:
    explicit HudMessageQueue(QObject* parent = nullptr);

    [[nodiscard]] bool visible() const noexcept;
    [[nodiscard]] QString messageKey() const;
    [[nodiscard]] QString valueText() const;

    static constexpr int holdDurationMs() noexcept { return 1200; }

    void showVolume(double volumePercent, bool muted);
    void showSeek(const QString& positionText);
    void showSeekFailure();
    void clear();

signals:
    void stateChanged();

private:
    enum class MessageKind {
        Volume,
        Seek,
    };

    struct Message final {
        MessageKind kind = MessageKind::Volume;
        QString key;
        QString value;
    };

    static constexpr qsizetype kMaximumPendingMessages = 3;

    void enqueueOrUpdate(Message message);
    void present(Message message);
    void advance();
    void restartTimer();
    void removePending(MessageKind kind);

    QTimer holdTimer_;
    std::optional<Message> current_;
    std::deque<Message> pending_;
};

} // namespace player::presentation
