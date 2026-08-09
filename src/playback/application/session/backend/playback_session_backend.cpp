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
#include <QtGlobal>

#include <utility>
#include <variant>

namespace player::playback::application {

PlaybackSessionBackend::PlaybackSessionBackend() = default;

PlaybackSessionBackend::~PlaybackSessionBackend()
{
    if (handle_ != nullptr || propertyObserver_ != nullptr || eventLoop_ != nullptr
        || commandExecutor_ != nullptr) {
        qFatal("PlaybackSessionBackend must be shut down on the playback thread before destruction.");
    }
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

    mediaGenerationAttributor_.reset();

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
            const player::playback::domain::MediaGeneration generation =
                mediaGenerationAttributor_.attribute(event);

            if (!eventHandler_) {
                return;
            }

            auto mapped = player::playback::mpv::MpvPlaybackEventMapper::map(event);
            if (mapped.has_value()) {
                mapped->generation = generation;
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
    eventHandler_ = {};

    if (eventLoop_ != nullptr) {
        eventLoop_->stop();
    }

    if (propertyObserver_ != nullptr) {
        QString ignored;
        (void)propertyObserver_->stop(&ignored);
    }

    commandExecutor_.reset();
    propertyObserver_.reset();
    eventLoop_.reset();

    if (handle_ != nullptr) {
        handle_->close();
        handle_.reset();
    }

    mediaGenerationAttributor_.reset();
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
    player::playback::domain::MediaGeneration generation,
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

    const bool isLoad = std::holds_alternative<player::playback::domain::LoadMediaCommand>(
        command.payload());
    if (isLoad) {
        if (!generation.isValid()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Load submission requires a valid MediaGeneration.");
            }
            return false;
        }
        mediaGenerationAttributor_.noteLoadSubmission(command.requestId(), generation);
    }

    const bool submitted = commandExecutor_->submit(
        command.requestId().value(),
        *request,
        errorMessage);
    if (!submitted && isLoad) {
        mediaGenerationAttributor_.cancelLoadSubmission(command.requestId());
    }
    return submitted;
}

} // namespace player::playback::application
