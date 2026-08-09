#pragma once

#include <QObject>

class QTimer;

namespace player::playback::application {

class RequestTracker;

class RequestTimeoutMonitor final : public QObject
{
public:
    explicit RequestTimeoutMonitor(RequestTracker& tracker, QObject* parent = nullptr);

    RequestTimeoutMonitor(const RequestTimeoutMonitor&) = delete;
    RequestTimeoutMonitor& operator=(const RequestTimeoutMonitor&) = delete;

    void start();
    void stop();
    [[nodiscard]] bool isRunning() const noexcept;

private:
    RequestTracker& tracker_;
    QTimer* timer_ = nullptr;
};

} // namespace player::playback::application
