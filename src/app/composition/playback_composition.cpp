#include "app/composition/playback_composition.h"

#include "foundation/logging/log_categories.h"
#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/requests/playback_request_id_generator.h"
#include "playback/application/session/playback_session_thread.h"
#include "playback/application/state_publisher/state_publisher.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/commands/transport_command.h"
#include "presentation/viewmodels/player/transport/player_transport_view_model.h"

#include <QLoggingCategory>
#include <QString>

namespace player::app {
namespace {

using player::playback::domain::PlaybackCommand;
using player::playback::domain::TransportAction;
using player::playback::domain::TransportCommand;

QString transportActionName(TransportAction action)
{
    switch (action) {
    case TransportAction::Play:
        return QStringLiteral("play");
    case TransportAction::Pause:
        return QStringLiteral("pause");
    case TransportAction::Stop:
        return QStringLiteral("stop");
    }
    return QStringLiteral("unknown");
}

} // namespace

PlaybackComposition::PlaybackComposition()
    : playbackThread_(
        std::make_unique<player::playback::application::PlaybackSessionThread>())
    , requestIdGenerator_(
        std::make_unique<player::playback::application::PlaybackRequestIdGenerator>())
    , transportViewModel_(
        std::make_unique<player::presentation::PlayerTransportViewModel>())
{
    auto* publisher = playbackThread_->statePublisher();
    QObject::connect(
        publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        transportViewModel_.get(),
        &player::presentation::PlayerTransportViewModel::acceptSnapshot);

    QObject::connect(
        transportViewModel_.get(),
        &player::presentation::PlayerTransportViewModel::playRequested,
        playbackThread_.get(),
        [this]() { submitPlay(); });
    QObject::connect(
        transportViewModel_.get(),
        &player::presentation::PlayerTransportViewModel::pauseRequested,
        playbackThread_.get(),
        [this]() { submitPause(); });
    QObject::connect(
        transportViewModel_.get(),
        &player::presentation::PlayerTransportViewModel::stopRequested,
        playbackThread_.get(),
        [this]() { submitStop(); });

    QObject::connect(
        playbackThread_.get(),
        &player::playback::application::PlaybackSessionThread::startupFailed,
        playbackThread_.get(),
        [](const QString& diagnostic) {
            qCCritical(player::logging::appLifecycle).noquote()
                << "Playback composition startup failed:" << diagnostic;
        });
}

PlaybackComposition::~PlaybackComposition()
{
    QString diagnostic;
    if (!stop(&diagnostic)) {
        qCCritical(player::logging::appLifecycle).noquote()
            << "Playback composition shutdown failed:" << diagnostic;
    }
}

bool PlaybackComposition::start(QString* errorMessage)
{
    return playbackThread_->start(errorMessage);
}

bool PlaybackComposition::stop(QString* errorMessage)
{
    return playbackThread_->stop(errorMessage);
}

bool PlaybackComposition::isRunning() const noexcept
{
    return playbackThread_->isRunning();
}

player::presentation::PlayerTransportViewModel&
PlaybackComposition::transportViewModel() noexcept
{
    return *transportViewModel_;
}

void PlaybackComposition::submitPlay()
{
    using namespace player::playback;
    application::PlaybackCommandBus* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction)
            << "Transport play intent ignored because PlaybackCommandBus is unavailable.";
        return;
    }

    QString diagnostic;
    const PlaybackCommand command{
        requestIdGenerator_->next(),
        TransportCommand{TransportAction::Play}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Transport command submission failed:"
            << transportActionName(TransportAction::Play)
            << diagnostic;
    }
}

void PlaybackComposition::submitPause()
{
    using namespace player::playback;
    application::PlaybackCommandBus* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction)
            << "Transport pause intent ignored because PlaybackCommandBus is unavailable.";
        return;
    }

    QString diagnostic;
    const PlaybackCommand command{
        requestIdGenerator_->next(),
        TransportCommand{TransportAction::Pause}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Transport command submission failed:"
            << transportActionName(TransportAction::Pause)
            << diagnostic;
    }
}

void PlaybackComposition::submitStop()
{
    using namespace player::playback;
    application::PlaybackCommandBus* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction)
            << "Transport stop intent ignored because PlaybackCommandBus is unavailable.";
        return;
    }

    QString diagnostic;
    const PlaybackCommand command{
        requestIdGenerator_->next(),
        TransportCommand{TransportAction::Stop}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Transport command submission failed:"
            << transportActionName(TransportAction::Stop)
            << diagnostic;
    }
}

} // namespace player::app
