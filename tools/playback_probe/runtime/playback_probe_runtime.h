#pragma once

#include "playback/infrastructure/mpv/commands/mpv_command_request.h"
#include "playback/infrastructure/mpv/events/mpv_event.h"

#include <QObject>
#include <QtGlobal>

#include <memory>

class QString;

namespace player::playback::mpv {
class MpvCommandExecutor;
class MpvEventLoop;
class MpvHandle;
class MpvPropertyObserver;
} // namespace player::playback::mpv

namespace player::tools::playback_probe {

class PlaybackProbeRuntime final : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackProbeRuntime(QObject* parent = nullptr);
    ~PlaybackProbeRuntime() override;

    PlaybackProbeRuntime(const PlaybackProbeRuntime&) = delete;
    PlaybackProbeRuntime& operator=(const PlaybackProbeRuntime&) = delete;

    [[nodiscard]] bool initialize(QString* errorMessage = nullptr);
    [[nodiscard]] bool submit(
        quint64 requestId,
        const player::playback::mpv::MpvCommandRequest& request,
        QString* errorMessage = nullptr);
    void shutdown() noexcept;

    [[nodiscard]] bool isReady() const noexcept;

signals:
    void eventDecoded(const player::playback::mpv::MpvEvent& event);

private:
    std::unique_ptr<player::playback::mpv::MpvHandle> handle_;
    std::unique_ptr<player::playback::mpv::MpvPropertyObserver> propertyObserver_;
    std::unique_ptr<player::playback::mpv::MpvEventLoop> eventLoop_;
    std::unique_ptr<player::playback::mpv::MpvCommandExecutor> commandExecutor_;
};

} // namespace player::tools::playback_probe
