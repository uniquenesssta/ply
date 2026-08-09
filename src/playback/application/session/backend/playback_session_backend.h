#pragma once

#include "playback/domain/commands/playback_command.h"
#include "playback/domain/events/playback_event.h"

#include <functional>
#include <memory>

class QString;

namespace player::playback::mpv {
class MpvCommandExecutor;
class MpvEventLoop;
class MpvHandle;
class MpvPropertyObserver;
} // namespace player::playback::mpv

namespace player::playback::application {

class PlaybackSessionBackend final
{
public:
    using EventHandler = std::function<void(const player::playback::domain::PlaybackEvent&)>;

    PlaybackSessionBackend();
    ~PlaybackSessionBackend();

    PlaybackSessionBackend(const PlaybackSessionBackend&) = delete;
    PlaybackSessionBackend& operator=(const PlaybackSessionBackend&) = delete;

    void setEventHandler(EventHandler handler);

    [[nodiscard]] bool initialize(QString* errorMessage = nullptr);
    void shutdown() noexcept;

    [[nodiscard]] bool isReady() const noexcept;
    [[nodiscard]] bool submit(
        const player::playback::domain::PlaybackCommand& command,
        QString* errorMessage = nullptr);

private:
    EventHandler eventHandler_;
    std::unique_ptr<player::playback::mpv::MpvHandle> handle_;
    std::unique_ptr<player::playback::mpv::MpvPropertyObserver> propertyObserver_;
    std::unique_ptr<player::playback::mpv::MpvEventLoop> eventLoop_;
    std::unique_ptr<player::playback::mpv::MpvCommandExecutor> commandExecutor_;
};

} // namespace player::playback::application
