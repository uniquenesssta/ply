#pragma once

#include "playback/application/requests/playback_request.h"

#include <QObject>

#include <functional>

class QTimer;

namespace player::playback::application {

class RequestTracker;

class RequestTimeoutMonitor final : public QObject
{
public:
    using TimeoutHandler = std::function<void(const PlaybackRequestRecord&)>;

    explicit RequestTimeoutMonitor(
        RequestTracker& tracker,
        TimeoutHandler timeoutHandler = {},
        QObject* parent = nullptr);

    RequestTimeoutMonitor(const RequestTimeoutMonitor&) = delete;
    RequestTimeoutMonitor& operator=(const RequestTimeoutMonitor&) = delete;

    void start();
    void stop();

private:
    RequestTracker& tracker_;
    TimeoutHandler timeoutHandler_;
    QTimer* timer_ = nullptr;
};

} // namespace player::playback::application
