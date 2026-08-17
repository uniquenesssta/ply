#include "app/composition/playback_composition.h"

#include "app/composition/render/player_video_render_binding.h"
#include "foundation/logging/log_categories.h"
#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/requests/playback_request.h"
#include "playback/application/requests/playback_request_id_generator.h"
#include "playback/application/session/playback_session_thread.h"
#include "playback/application/state_publisher/state_publisher.h"
#include "playback/domain/commands/load_media_command.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/commands/volume_command.h"
#include "presentation/viewmodels/player/hud/hud_message_queue.h"
#include "presentation/viewmodels/player/media/player_media_view_model.h"
#include "presentation/viewmodels/player/status/player_status_view_model.h"
#include "presentation/viewmodels/player/timeline/player_timeline_view_model.h"
#include "presentation/viewmodels/player/transport/player_transport_view_model.h"
#include "presentation/viewmodels/player/volume/player_volume_view_model.h"

#include <QLoggingCategory>
#include <QObject>
#include <QString>

namespace player::app {
namespace {

using player::playback::domain::SeekMode;
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

QString seekModeName(SeekMode mode)
{
    switch (mode) {
    case SeekMode::Absolute:
        return QStringLiteral("absolute");
    case SeekMode::Relative:
        return QStringLiteral("relative");
    }
    return QStringLiteral("unknown");
}

bool isSeekRequestType(quint8 requestType) noexcept
{
    using player::playback::application::PlaybackRequestType;
    return requestType == static_cast<quint8>(PlaybackRequestType::SeekAbsolute)
        || requestType == static_cast<quint8>(PlaybackRequestType::SeekRelative);
}

} // namespace

PlaybackComposition::PlaybackComposition()
    : playbackThread_(
        std::make_unique<player::playback::application::PlaybackSessionThread>())
    , videoRenderBinding_(
        std::make_unique<PlayerVideoRenderBinding>(*playbackThread_))
    , requestIdGenerator_(
        std::make_unique<player::playback::application::PlaybackRequestIdGenerator>())
    , transportViewModel_(
        std::make_unique<player::presentation::PlayerTransportViewModel>())
    , timelineViewModel_(
        std::make_unique<player::presentation::PlayerTimelineViewModel>())
    , volumeViewModel_(
        std::make_unique<player::presentation::PlayerVolumeViewModel>())
    , statusViewModel_(
        std::make_unique<player::presentation::PlayerStatusViewModel>())
    , mediaViewModel_(
        std::make_unique<player::presentation::PlayerMediaViewModel>())
    , hudMessageQueue_(
        std::make_unique<player::presentation::HudMessageQueue>())
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
        publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        statusViewModel_.get(),
        &player::presentation::PlayerStatusViewModel::acceptSnapshot);
    QObject::connect(
        publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        mediaViewModel_.get(),
        &player::presentation::PlayerMediaViewModel::acceptSnapshot);

    QObject::connect(
        playbackThread_.get(),
        &player::playback::application::PlaybackSessionThread::requestFailed,
        timelineViewModel_.get(),
        [this](quint8 requestType, const QString& diagnostic) {
            if (!isSeekRequestType(requestType)) {
                return;
            }

            const bool cleared = timelineViewModel_->rejectPendingSeek();
            if (cleared) {
                hudMessageQueue_->showSeekFailure();
            }
            qCWarning(player::logging::uiInteraction).noquote()
                << "Tracked timeline seek request failed:"
                << diagnostic;
        });

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
            if (!submitSeek(absoluteSeconds, SeekMode::Absolute)) {
                if (timelineViewModel_->rejectPendingSeek()) {
                    hudMessageQueue_->showSeekFailure();
                }
                return;
            }
            hudMessageQueue_->showSeek(timelineViewModel_->positionText());
        });
    QObject::connect(
        timelineViewModel_.get(),
        &player::presentation::PlayerTimelineViewModel::relativeSeekRequested,
        playbackThread_.get(),
        [this](double deltaSeconds) {
            if (!submitSeek(deltaSeconds, SeekMode::Relative)) {
                if (timelineViewModel_->rejectPendingSeek()) {
                    hudMessageQueue_->showSeekFailure();
                }
                return;
            }
            hudMessageQueue_->showSeek(timelineViewModel_->positionText());
        });
    QObject::connect(
        volumeViewModel_.get(),
        &player::presentation::PlayerVolumeViewModel::volumeRequested,
        playbackThread_.get(),
        [this](double percent) {
            if (!submitVolume(percent)) {
                (void)volumeViewModel_->rejectPendingVolume();
                return;
            }
            hudMessageQueue_->showVolume(
                volumeViewModel_->volumePercent(),
                volumeViewModel_->muted());
        });
    QObject::connect(
        volumeViewModel_.get(),
        &player::presentation::PlayerVolumeViewModel::mutedRequested,
        playbackThread_.get(),
        [this](bool muted) {
            if (!submitMuted(muted)) {
                (void)volumeViewModel_->rejectPendingMute();
                return;
            }
            hudMessageQueue_->showVolume(
                volumeViewModel_->volumePercent(),
                volumeViewModel_->muted());
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
    if (playbackThread_->isRunning()) {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    if (!videoRenderBinding_->resetForStart(errorMessage)) {
        return false;
    }
    return playbackThread_->start(errorMessage);
}

bool PlaybackComposition::stop(QString* errorMessage)
{
    beginVideoRenderShutdown();
    if (!videoRenderBinding_->waitForRenderRelease(errorMessage)) {
        return false;
    }

    hudMessageQueue_->clear();
    return playbackThread_->stop(errorMessage);
}

bool PlaybackComposition::isRunning() const noexcept
{
    return playbackThread_->isRunning();
}

bool PlaybackComposition::attachVideoOutput(QObject* qmlRoot, QString* errorMessage)
{
    return videoRenderBinding_->attachRoot(qmlRoot, errorMessage);
}

void PlaybackComposition::beginVideoRenderShutdown() noexcept
{
    videoRenderBinding_->beginShutdown();
}

player::playback::application::StatePublisher&
PlaybackComposition::statePublisher() noexcept
{
    return *playbackThread_->statePublisher();
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

player::presentation::PlayerStatusViewModel&
PlaybackComposition::statusViewModel() noexcept
{
    return *statusViewModel_;
}

player::presentation::PlayerMediaViewModel&
PlaybackComposition::mediaViewModel() noexcept
{
    return *mediaViewModel_;
}

player::presentation::HudMessageQueue&
PlaybackComposition::hudMessageQueue() noexcept
{
    return *hudMessageQueue_;
}

bool PlaybackComposition::submitMediaLoad(const QString& canonicalSource)
{
    if (canonicalSource.isEmpty()) {
        return false;
    }

    auto* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction)
            << "Media open intent ignored because PlaybackCommandBus is unavailable";
        return false;
    }

    QString diagnostic;
    const player::playback::domain::PlaybackCommand command{
        requestIdGenerator_->next(),
        player::playback::domain::LoadMediaCommand{canonicalSource}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Media load command submission failed:" << diagnostic;
        return false;
    }

    return true;
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

bool PlaybackComposition::submitSeek(double seconds, SeekMode mode)
{
    auto* bus = playbackThread_->commandBus();
    if (bus == nullptr || !bus->isAcceptingCommands()) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Timeline seek intent ignored because PlaybackCommandBus is unavailable:"
            << seekModeName(mode)
            << seconds;
        return false;
    }

    QString diagnostic;
    const player::playback::domain::PlaybackCommand command{
        requestIdGenerator_->next(),
        player::playback::domain::SeekCommand{seconds, mode}};
    if (!bus->submit(command, &diagnostic)) {
        qCWarning(player::logging::uiInteraction).noquote()
            << "Timeline seek command submission failed:"
            << seekModeName(mode)
            << seconds
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
