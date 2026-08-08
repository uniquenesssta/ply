#include "playback_probe_runtime.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/commands/mpv_command_executor.h"
#include "playback/infrastructure/mpv/events/mpv_event_loop.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/initialization/mpv_option_profile.h"
#include "playback/infrastructure/mpv/properties/mpv_property_observer.h"

#include <QByteArray>
#include <QString>

namespace player::tools::playback_probe {

PlaybackProbeRuntime::PlaybackProbeRuntime(QObject* parent)
    : QObject(parent)
{
}

PlaybackProbeRuntime::~PlaybackProbeRuntime()
{
    shutdown();
}

bool PlaybackProbeRuntime::initialize(QString* errorMessage)
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
            *errorMessage = QStringLiteral("Playback probe runtime is partially initialized.");
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

    const player::playback::mpv::MpvOptionProfile probeProfile(
        QList<player::playback::mpv::MpvOption>{
            {QByteArrayLiteral("config"), QByteArrayLiteral("no")},
            {QByteArrayLiteral("vo"), QByteArrayLiteral("null")},
            {QByteArrayLiteral("ao"), QByteArrayLiteral("null")},
        });

    if (!player::playback::mpv::MpvInitializer::initialize(*handle_, probeProfile, &error)) {
        shutdown();
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    propertyObserver_ = std::make_unique<player::playback::mpv::MpvPropertyObserver>(*handle_);
    eventLoop_ = std::make_unique<player::playback::mpv::MpvEventLoop>(*handle_);
    commandExecutor_ = std::make_unique<player::playback::mpv::MpvCommandExecutor>(*handle_);

    connect(
        eventLoop_.get(),
        &player::playback::mpv::MpvEventLoop::eventDecoded,
        this,
        &PlaybackProbeRuntime::eventDecoded);

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

bool PlaybackProbeRuntime::submit(
    quint64 requestId,
    const player::playback::mpv::MpvCommandRequest& request,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!isReady()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback probe runtime is not initialized.");
        }
        return false;
    }

    return commandExecutor_->submit(requestId, request, errorMessage);
}

void PlaybackProbeRuntime::shutdown() noexcept
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

bool PlaybackProbeRuntime::isReady() const noexcept
{
    return handle_ != nullptr && handle_->isOpen() && handle_->isInitialized()
        && propertyObserver_ != nullptr && propertyObserver_->isObserving()
        && eventLoop_ != nullptr && eventLoop_->isRunning() && commandExecutor_ != nullptr;
}

} // namespace player::tools::playback_probe
