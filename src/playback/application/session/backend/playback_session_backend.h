#pragma once

#include "mpv_media_generation_attributor.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/events/playback_event.h"
#include "playback/domain/state/media_generation.h"

#include <QtGlobal>

#include <functional>
#include <memory>

class QString;

namespace player::playback::mpv {
class MpvCommandExecutor;
class MpvEventLoop;
class MpvHandle;
class MpvPropertyObserver;
class MpvPropertyReader;
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
    [[nodiscard]] quintptr renderCoreAddress() noexcept;
    [[nodiscard]] bool submit(
        const player::playback::domain::PlaybackCommand& command,
        player::playback::domain::MediaGeneration generation,
        QString* errorMessage = nullptr);
    void refreshExternalSubtitleTrackState(
        player::playback::domain::MediaGeneration generation);

private:
    void refreshCurrentMediaProperties(
        player::playback::domain::MediaGeneration generation);

    EventHandler eventHandler_;
    MpvMediaGenerationAttributor mediaGenerationAttributor_;
    std::unique_ptr<player::playback::mpv::MpvHandle> handle_;
    std::unique_ptr<player::playback::mpv::MpvPropertyObserver> propertyObserver_;
    std::unique_ptr<player::playback::mpv::MpvPropertyReader> propertyReader_;
    std::unique_ptr<player::playback::mpv::MpvEventLoop> eventLoop_;
    std::unique_ptr<player::playback::mpv::MpvCommandExecutor> commandExecutor_;
};

} // namespace player::playback::application
