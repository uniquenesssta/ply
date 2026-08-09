#include "playback_session_backend.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/commands/mpv_command_executor.h"
#include "playback/infrastructure/mpv/commands/mpv_playback_command_mapper.h"
#include "playback/infrastructure/mpv/events/mpv_event_loop.h"
#include "playback/infrastructure/mpv/events/mpv_playback_event_mapper.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/properties/mpv_property_observer.h"

#include <QObject>
#include <QString>

#include <utility>

namespace player::playback::application {

PlaybackSessionBackend::PlaybackSessionBackend() = default;

PlaybackSessionBackend::~PlaybackSessionBackend()
{
    shutdown();
}

void PlaybackSessionBackend::setEventHandler(EventHandler handler)
{
    eventHandler_ = std::move(handler);
}

bool PlaybackSessionBackend::initialize(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (isReady()) {
        return true;
    }

    if (handle_ != nullptr || propertyObserver_ != nullptr || eventLoop_ != nullptr
        || commandExecutor_ != nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback session backend is partially initialized.");
        }
        return false;
    }

    QString error;
    handle_ = player::playback::mpv::MpvHandle::create(&error);
    if (handle_ == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    if (!player::playback::mpv::MpvInitializer::initializeProduct(*handle_, &error)) {
        shutdown();
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    propertyObserver_ = std::make_unique<player::playback::mpv::MpvPropertyObserver>(*handle_);
    eventLoop_ = std::make_unique<player::playback::mpv::MpvEventLoop>(*handle_);
    commandExecutor_ = std::make_unique<player::playback::mpv::MpvCommandExecutor>(*handle_);

    QObject::connect(
        eventLoop_.get(),
        &player::playback::mpv::MpvEventLoop::eventDecoded,
        eventLoop_.get(),
        [this](const player::playback::mpv::MpvEvent& event) {
            if (!eventHandler_) {
                return;
            }
            const auto mapped = player::playback::mpv::MpvPlaybackEventMapper::map(event);
            if (mapped.has_value()) {
                eventHandler_(*mapped);
            }
        });

    if (!eventLoop_->start(&error)) {
        shutdown();
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    if (!propertyObserver_->start(&error)) {
        shutdown();
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    return true;
}

void PlaybackSessionBackend::shutdown() noexcept
{
    if (propertyObserver_ != nullptr) {
        QString ignored;
        (void)propertyObserver_->stop(&ignored);
    }

    if (eventLoop_ != nullptr) {
        eventLoop_->stop();
    }

    commandExecutor_.reset();
    propertyObserver_.reset();
    eventLoop_.reset();

    if (handle_ != nullptr) {
        handle_->close();
        handle_.reset();
    }
}

bool PlaybackSessionBackend::isReady() const noexcept
{
    return handle_ != nullptr && handle_->isOpen() && handle_->isInitialized()
        && propertyObserver_ != nullptr && propertyObserver_->isObserving()
        && eventLoop_ != nullptr && eventLoop_->isRunning()
        && commandExecutor_ != nullptr;
}

bool PlaybackSessionBackend::submit(
    const player::playback::domain::PlaybackCommand& command,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!isReady()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback session backend is not initialized.");
        }
        return false;
    }

    const auto request = player::playback::mpv::MpvPlaybackCommandMapper::map(command.payload());
    if (!request.has_value()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback lifecycle commands are owned by PlaybackSession.");
        }
        return false;
    }

    return commandExecutor_->submit(command.requestId().value(), *request, errorMessage);
}

} // namespace player::playback::application
