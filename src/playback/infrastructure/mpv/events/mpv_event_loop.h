#pragma once

#include <QObject>
#include <QtGlobal>

class QString;

namespace player::playback::mpv {

class MpvHandle;
class MpvWakeupBridge;

class MpvEventLoop final : public QObject
{
    Q_OBJECT

public:
    explicit MpvEventLoop(MpvHandle& handle, QObject* parent = nullptr);
    ~MpvEventLoop() override;

    MpvEventLoop(const MpvEventLoop&) = delete;
    MpvEventLoop& operator=(const MpvEventLoop&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);
    void stop() noexcept;

    [[nodiscard]] bool isRunning() const noexcept;

signals:
    void eventDrained(int eventId, quint64 replyUserdata, int error);

private slots:
    void drainPendingEvents();

private:
    [[nodiscard]] bool isOnOwningThread() const noexcept;

    MpvHandle& handle_;
    MpvWakeupBridge* wakeupBridge_ = nullptr;
    bool running_ = false;
};

} // namespace player::playback::mpv
