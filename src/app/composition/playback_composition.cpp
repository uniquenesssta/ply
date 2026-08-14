#include "app/composition/playback_composition.h"

#include "foundation/logging/log_categories.h"
#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/requests/playback_request_id_generator.h"
#include "playback/application/session/playback_session_thread.h"
#include "playback/application/state_publisher/state_publisher.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/volume_command.h"
#include "presentation/viewmodels/player/timeline/player_timeline_view_model.h"
#include "presentation/viewmodels/player/transport/player_transport_view_model.h"
#include "presentation/viewmodels/player/volume/player_volume_view_model.h"

#include <QLoggingCategory>
#include <QObject>
#include <QString>

namespace player::app {
namespace {

using player::playback::domain::TransportAction;

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
    , timelineViewModel_(
        std::make_unique<player::presentation::PlayerTimelineViewModel>())
    , volumeViewModel_(
        std::make_unique<player::presentation::PlayerVolumeViewModel>())
{
    auto* publisher = playbackThread_->statePublisher();
    QObject::connect(
        publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        transportViewModel_.get(),
        &player::presentation::PlayerTransportViewModel::acceptSnapshot);
    QObject::connect(
        publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        timelineViewModel_.get(),
        &player::presentation::PlayerTimelineViewModel::acceptSnapshot);
    QObject::connect(
        publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        volumeViewModel_.get(),
        &player::presentation::PlayerVolumeViewModel::acceptSnapshot);

    QObject::connect(
        transportViewModel_.get(),
        &player::presentation::PlayerTransportViewModel::playRequested,
        playbackThread_.get(),
        [this]() { submitTransport(TransportAction::Play); });
    QObject::connect(
        transportViewModel_.get(),
        &player::presentation::PlayerTransportViewModel::pauseRequested,
        playbackThread_.get(),
        [this]() { submitTransport(TransportAction::Pause); });
    QObject::connect(
        transportViewModel_.get(),
        &player::presentation::PlayerTransportViewModel::stopRequested,
        playbackThread_.get(),
        [this]() { submitTransport(TransportAction::Stop); });
    QObject::connect(
        timelineViewModel_.get(),
        &player::presentation::PlayerTimelineViewModel::seekRequested,
        playbackThread_.get(),
        [this](double absoluteSeconds) {
            if (!submitSeek(absoluteSeconds)) {
                (void)timelineViewModel_->rejectPendingSeek();
            }
        });
    QObject::connect(
        volumeViewModel_.get(),
        &player::presentation::PlayerVolumeViewModel::volumeRequested,
        playbackThread_.get(),
        [this](double percent) {
            if (!submitVolume(percent)) {
                (void)volumeViewModel_->rejectPendingVolume();
            }
        });
    QObject::connect(
        volumeViewModel_.get(),
        &player::presentation::PlayerVolumeViewModel::mutedRequested,
        playbackThread_.get(),
        [this](bool muted) {
            if (!submitMuted(muted)) {
                (void)volumeViewModel_->rejectPendingMute();
            }
        });

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

player::presentation::PlayerTimelineViewModel&
PlaybackComposition::timelineViewModel() noexcept
{
    return *timelineViewModel_;
}

player::presentation::PlayerVolumeViewModel&
PlaybackComposition::volumeViewModel() noexcept
{
    return *volumeViewModel_;
}

void PlaybackComposition::submitTransport(TransportAction action)
{
    auto* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Transport intent ignored because PlaybackCommandBus is unavailable:"
            << transportActionName(action);
        return;
    }

    QString diagnostic;
    const player::playback::domain::PlaybackCommand command{
        requestIdGenerator_->next(),
        player::playback::domain::TransportCommand{action}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Transport command submission failed:"
            << transportActionName(action)
            << diagnostic;
    }
}

bool PlaybackComposition::submitSeek(double absoluteSeconds)
{
    auto* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Timeline seek intent ignored because PlaybackCommandBus is unavailable:"
            << absoluteSeconds;
        return false;
    }

    QString diagnostic;
    const player::playback::domain::PlaybackCommand command{
        requestIdGenerator_->next(),
        player::playback::domain::SeekCommand{
            absoluteSeconds,
            player::playback::domain::SeekMode::Absolute}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Timeline seek command submission failed:"
            << absoluteSeconds
            << diagnostic;
        return false;
    }

    return true;
}

bool PlaybackComposition::submitVolume(double percent)
{
    auto* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Volume intent ignored because PlaybackCommandBus is unavailable:"
            << percent;
        return false;
    }

    QString diagnostic;
    const player::playback::domain::PlaybackCommand command{
        requestIdGenerator_->next(),
        player::playback::domain::SetVolumeCommand{percent}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Volume command submission failed:"
            << percent
            << diagnostic;
        return false;
    }

    return true;
}

bool PlaybackComposition::submitMuted(bool muted)
{
    auto* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Mute intent ignored because PlaybackCommandBus is unavailable:"
            << muted;
        return false;
    }

    QString diagnostic;
    const player::playback::domain::PlaybackCommand command{
        requestIdGenerator_->next(),
        player::playback::domain::SetMutedCommand{muted}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Mute command submission failed:"
            << muted
            << diagnostic;
        return false;
    }

    return true;
}

} // namespace player::app
