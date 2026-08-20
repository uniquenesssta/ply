#include "app/composition/application_container.h"

#include "app/bootstrap/logging_bootstrap.h"
#include "app/bootstrap/qml_bootstrap.h"
#include "app/composition/playback_composition.h"
#include "foundation/logging/log_categories.h"
#include "media/application/arguments/media_argument_open_workflow.h"
#include "media/application/drop/media_drop_handler.h"
#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"
#include "playback/application/requests/playback_request.h"
#include "playback/application/state_publisher/state_publisher.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"
#include "playlist/application/playlist_advance_arbiter.h"
#include "playlist/application/playlist_auto_advance.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/application/playlist_mutation.h"
#include "playlist/domain/playlist.h"
#include "playlist/presentation/playlist_entry_playback_state.h"
#include "playlist/presentation/playlist_list_model.h"
#include "presentation/viewmodels/player/hud/hud_message_queue.h"
#include "tracks/application/audio_delay_controller.h"
#include "tracks/application/external_subtitle_loader.h"
#include "tracks/application/subtitle_delay_controller.h"
#include "tracks/application/track_selection_controller.h"
#include "tracks/presentation/chapter_model.h"
#include "tracks/presentation/track_list_model.h"

#include <QList>
#include <QLoggingCategory>
#include <QObject>
#include <QString>

#include <utility>

namespace player::app {

ApplicationContainer::ApplicationContainer(RuntimePaths runtimePaths)
    : ApplicationContainer(
        std::move(runtimePaths),
        std::make_unique<LoggingBootstrap>())
{
}

ApplicationContainer::ApplicationContainer(
    RuntimePaths runtimePaths,
    std::unique_ptr<LoggingBootstrap> loggingBootstrap)
    : runtimePaths_(std::move(runtimePaths))
    , loggingBootstrap_(std::move(loggingBootstrap))
    , playbackComposition_(std::make_unique<PlaybackComposition>())
    , playlist_(std::make_unique<player::playlist::domain::Playlist>())
    , playlistMutation_(
        std::make_unique<player::playlist::application::PlaylistMutation>(*playlist_))
    , playlistAdvanceArbiter_(
        std::make_unique<player::playlist::application::PlaylistAdvanceArbiter>())
    , playlistController_(
        std::make_unique<player::playlist::application::PlaylistController>(
            *playlist_,
            *playlistMutation_,
            [this](const player::media::domain::MediaSource& source) {
                return playbackComposition_ != nullptr
                    && playbackComposition_->submitMediaLoad(source.location());
            },
            [this]() {
                return playbackComposition_ != nullptr
                    && playbackComposition_->submitMediaStop();
            },
            nullptr,
            playlistAdvanceArbiter_.get()))
    , playlistEntryPlaybackState_(
        std::make_unique<player::playlist::presentation::PlaylistEntryPlaybackState>(
            *playlistController_))
    , playlistAutoAdvance_(
        std::make_unique<player::playlist::application::PlaylistAutoAdvance>(
            *playlist_,
            *playlistController_,
            *playlistAdvanceArbiter_))
    , playlistListModel_(
        std::make_unique<player::playlist::presentation::PlaylistListModel>(
            *playlistController_,
            *playlistEntryPlaybackState_))
    , trackSelectionController_(
        std::make_unique<player::tracks::application::TrackSelectionController>(
            [this](const player::playback::domain::TrackSelectionCommand& selection) {
                return playbackComposition_ != nullptr
                    && playbackComposition_->submitTrackSelection(selection);
            }))
    , externalSubtitleLoader_(
        std::make_unique<player::tracks::application::ExternalSubtitleLoader>(
            [this](const player::playback::domain::AddExternalSubtitleCommand& subtitle) {
                return playbackComposition_ != nullptr
                    && playbackComposition_->submitExternalSubtitle(subtitle);
            }))
    , subtitleDelayController_(
        std::make_unique<player::tracks::application::SubtitleDelayController>(
            [this](const player::playback::domain::SetSubtitleDelayCommand& delay) {
                return playbackComposition_ != nullptr
                    && playbackComposition_->submitSubtitleDelay(delay);
            }))
    , audioDelayController_(
        std::make_unique<player::tracks::application::AudioDelayController>(
            [this](const player::playback::domain::SetAudioDelayCommand& delay) {
                return playbackComposition_ != nullptr
                    && playbackComposition_->submitAudioDelay(delay);
            }))
    , audioTrackListModel_(
        std::make_unique<player::tracks::presentation::TrackListModel>(
            player::playback::domain::TrackKind::Audio))
    , subtitleTrackListModel_(
        std::make_unique<player::tracks::presentation::TrackListModel>(
            player::playback::domain::TrackKind::Subtitle))
    , chapterModel_(std::make_unique<player::tracks::presentation::ChapterModel>())
    , mediaOpenCoordinator_(
        std::make_unique<player::media::application::MediaOpenCoordinator>(
            [this](const player::media::domain::MediaSource& source) {
                return playlistController_ != nullptr
                    && playlistController_->openSource(source);
            },
            [this](const QList<player::media::domain::MediaSource>& sources) {
                return playlistController_ != nullptr
                    && playlistController_->openSources(sources);
            }))
    , urlOpenWorkflow_(
        std::make_unique<player::media::application::UrlOpenWorkflow>(
            *mediaOpenCoordinator_))
    , mediaArgumentOpenWorkflow_(
        std::make_unique<player::media::application::MediaArgumentOpenWorkflow>(
            *mediaOpenCoordinator_,
            *urlOpenWorkflow_))
    , mediaDropHandler_(
        std::make_unique<player::media::application::MediaDropHandler>(
            *mediaOpenCoordinator_,
            *urlOpenWorkflow_))
    , qmlBootstrap_(std::make_unique<QmlBootstrap>())
{
    if (loggingBootstrap_ == nullptr) {
        loggingBootstrap_ = std::make_unique<LoggingBootstrap>();
    }

    playbackComposition_->setPlaybackSupersessionObserver([this]() {
        if (playlistAdvanceArbiter_ != nullptr) {
            playlistAdvanceArbiter_->suppressObservedGeneration();
        }
    });
    playbackComposition_->setRequestFailureObserver(
        [this](quint8 requestType, const QString& diagnostic) {
            using player::playback::application::PlaybackRequestType;

            if (requestType == static_cast<quint8>(PlaybackRequestType::SetSubtitleDelay)
                && subtitleDelayController_ != nullptr
                && subtitleDelayController_->rejectPendingDelay()) {
                qCWarning(player::logging::uiInteraction).noquote()
                    << "Tracked subtitle delay request failed:"
                    << diagnostic;
            }

            if (requestType == static_cast<quint8>(PlaybackRequestType::SetAudioDelay)
                && audioDelayController_ != nullptr
                && audioDelayController_->rejectPendingDelay()) {
                qCWarning(player::logging::uiInteraction).noquote()
                    << "Tracked audio delay request failed:"
                    << diagnostic;
            }
        });

    auto& publisher = playbackComposition_->statePublisher();
    QObject::connect(
        &publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        audioTrackListModel_.get(),
        &player::tracks::presentation::TrackListModel::acceptSnapshot);
    QObject::connect(
        &publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        subtitleTrackListModel_.get(),
        &player::tracks::presentation::TrackListModel::acceptSnapshot);
    QObject::connect(
        &publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        chapterModel_.get(),
        &player::tracks::presentation::ChapterModel::acceptSnapshot);
    QObject::connect(
        &publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        subtitleDelayController_.get(),
        &player::tracks::application::SubtitleDelayController::acceptSnapshot);
    QObject::connect(
        subtitleDelayController_.get(),
        &player::tracks::application::SubtitleDelayController::delayConfirmed,
        subtitleDelayController_.get(),
        [this](int milliseconds) {
            if (playbackComposition_ != nullptr) {
                playbackComposition_->hudMessageQueue().showSubtitleDelay(milliseconds);
            }
        });
    QObject::connect(
        &publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        audioDelayController_.get(),
        &player::tracks::application::AudioDelayController::acceptSnapshot);
    QObject::connect(
        audioDelayController_.get(),
        &player::tracks::application::AudioDelayController::delayConfirmed,
        audioDelayController_.get(),
        [this](int milliseconds) {
            if (playbackComposition_ != nullptr) {
                playbackComposition_->hudMessageQueue().showAudioDelay(milliseconds);
            }
        });
    QObject::connect(
        &publisher,
        &player::playback::application::StatePublisher::snapshotPublished,
        &publisher,
        [this](const player::playback::domain::PlaybackSnapshot& snapshot) {
            if (playlistEntryPlaybackState_ != nullptr) {
                playlistEntryPlaybackState_->acceptPlaybackSnapshot(snapshot);
            }

            if (playlistAutoAdvance_ == nullptr) {
                return;
            }

            const quint64 mediaGeneration = snapshot.generation().value();
            const auto lifecycle = snapshot.lifecycle();
            playlistAutoAdvance_->acceptPlaybackState(
                mediaGeneration,
                lifecycle == player::playback::domain::PlaybackLifecycleState::Ended);
            if (lifecycle == player::playback::domain::PlaybackLifecycleState::Failed) {
                playlistAutoAdvance_->acceptPlaybackFailure(mediaGeneration);
            }
        });
}

ApplicationContainer::~ApplicationContainer()
{
    shutdown();
}

const RuntimePaths& ApplicationContainer::runtimePaths() const noexcept
{
    return runtimePaths_;
}

LoggingBootstrap& ApplicationContainer::loggingBootstrap() noexcept
{
    return *loggingBootstrap_;
}

PlaybackComposition& ApplicationContainer::playbackComposition() noexcept
{
    return *playbackComposition_;
}

player::playlist::application::PlaylistController&
ApplicationContainer::playlistController() noexcept
{
    return *playlistController_;
}

player::playlist::presentation::PlaylistListModel&
ApplicationContainer::playlistListModel() noexcept
{
    return *playlistListModel_;
}

player::tracks::application::TrackSelectionController&
ApplicationContainer::trackSelectionController() noexcept
{
    return *trackSelectionController_;
}

player::tracks::application::ExternalSubtitleLoader&
ApplicationContainer::externalSubtitleLoader() noexcept
{
    return *externalSubtitleLoader_;
}

player::tracks::application::SubtitleDelayController&
ApplicationContainer::subtitleDelayController() noexcept
{
    return *subtitleDelayController_;
}

player::tracks::application::AudioDelayController&
ApplicationContainer::audioDelayController() noexcept
{
    return *audioDelayController_;
}

player::tracks::presentation::TrackListModel&
ApplicationContainer::audioTrackListModel() noexcept
{
    return *audioTrackListModel_;
}

player::tracks::presentation::TrackListModel&
ApplicationContainer::subtitleTrackListModel() noexcept
{
    return *subtitleTrackListModel_;
}

player::tracks::presentation::ChapterModel&
ApplicationContainer::chapterModel() noexcept
{
    return *chapterModel_;
}

player::media::application::MediaOpenCoordinator&
ApplicationContainer::mediaOpenCoordinator() noexcept
{
    return *mediaOpenCoordinator_;
}

player::media::application::UrlOpenWorkflow&
ApplicationContainer::urlOpenWorkflow() noexcept
{
    return *urlOpenWorkflow_;
}

player::media::application::MediaArgumentOpenWorkflow&
ApplicationContainer::mediaArgumentOpenWorkflow() noexcept
{
    return *mediaArgumentOpenWorkflow_;
}

player::media::application::MediaDropHandler&
ApplicationContainer::mediaDropHandler() noexcept
{
    return *mediaDropHandler_;
}

QmlBootstrap& ApplicationContainer::qmlBootstrap() noexcept
{
    return *qmlBootstrap_;
}

void ApplicationContainer::shutdown() noexcept
{
    // Freeze the R4 render callback before destroying QML, then destroy the
    // video item/scene graph while the playback-owned mpv core is still alive.
    // PlaybackComposition::stop() subsequently proves render release before it
    // is allowed to stop PlaybackSession and destroy that core.
    if (playbackComposition_ != nullptr) {
        playbackComposition_->beginVideoRenderShutdown();
    }

    if (mediaOpenCoordinator_ != nullptr) {
        mediaOpenCoordinator_->beginShutdown();
    }

    qmlBootstrap_.reset();
    mediaDropHandler_.reset();
    mediaArgumentOpenWorkflow_.reset();
    urlOpenWorkflow_.reset();
    mediaOpenCoordinator_.reset();
    if (playbackComposition_ != nullptr) {
        playbackComposition_->setRequestFailureObserver({});
    }
    audioDelayController_.reset();
    subtitleDelayController_.reset();
    externalSubtitleLoader_.reset();
    trackSelectionController_.reset();
    chapterModel_.reset();
    subtitleTrackListModel_.reset();
    audioTrackListModel_.reset();
    playlistListModel_.reset();
    playlistEntryPlaybackState_.reset();
    if (playbackComposition_ != nullptr) {
        playbackComposition_->setPlaybackSupersessionObserver({});
    }
    playlistAutoAdvance_.reset();
    playlistController_.reset();
    playlistAdvanceArbiter_.reset();
    playlistMutation_.reset();
    playlist_.reset();

    if (playbackComposition_ != nullptr) {
        QString diagnostic;
        if (!playbackComposition_->stop(&diagnostic)) {
            qCCritical(player::logging::appLifecycle).noquote()
                << "Playback composition shutdown failed:" << diagnostic;
        }
        playbackComposition_.reset();
    }

    if (loggingBootstrap_ != nullptr) {
        loggingBootstrap_->stop();
        loggingBootstrap_.reset();
    }
}

} // namespace player::app
