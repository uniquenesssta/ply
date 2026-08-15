#pragma once

#include <QObject>
#include <QTimer>

namespace player::presentation {

class TimelineRelativeSeekCoalescer final : public QObject
{
    Q_OBJECT

public:
    explicit TimelineRelativeSeekCoalescer(QObject* parent = nullptr);

    [[nodiscard]] bool enqueue(double deltaSeconds);
    [[nodiscard]] bool clear();
    [[nodiscard]] bool hasPending() const noexcept;
    [[nodiscard]] double pendingDeltaSeconds() const noexcept;

signals:
    void flushRequested(double deltaSeconds);

private slots:
    void flushPending();

private:
    static constexpr int kCoalesceWindowMilliseconds = 100;
    static constexpr double kZeroTolerance = 0.0000001;

    QTimer timer_;
    double pendingDeltaSeconds_ = 0.0;
};

} // namespace player::presentation
