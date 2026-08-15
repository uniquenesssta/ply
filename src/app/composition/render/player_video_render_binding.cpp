#include "player_video_render_binding.h"

#include "playback/application/session/playback_session_thread.h"
#include "playback/infrastructure/mpv/render/mpv_render_shutdown_coordinator.h"
#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <chrono>

namespace player::app {
namespace {

constexpr std::chrono::milliseconds kRenderReleaseTimeout{5000};

} // namespace

PlayerVideoRenderBinding::PlayerVideoRenderBinding(
    player::playback::application::PlaybackSessionThread& playbackThread,
    QObject* parent)
    : QObject(parent)
{
    QObject::connect(
        &playbackThread,
        &player::playback::application::PlaybackSessionThread::renderCoreReady,
        this,
        [this](quintptr nativeHandle) {
            if (shutdownStarted_) {
                return;
            }
            coreAddress_ = nativeHandle;
            bindCoreIfReady();
        });
}

PlayerVideoRenderBinding::~PlayerVideoRenderBinding()
{
    if (shutdownCoordinator_ != nullptr
        && !shutdownCoordinator_->snapshot().renderReleased) {
        qFatal("PlayerVideoRenderBinding cannot be destroyed before video render resources are released.");
    }
}

bool PlayerVideoRenderBinding::resetForStart(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (shutdownCoordinator_ != nullptr
        && !shutdownCoordinator_->snapshot().renderReleased) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "Previous video render resources have not been released.");
        }
        return false;
    }

    videoItem_.clear();
    shutdownCoordinator_.reset();
    coreAddress_ = 0;
    shutdownStarted_ = false;
    return true;
}

bool PlayerVideoRenderBinding::attachRoot(QObject* qmlRoot, QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (shutdownStarted_) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "Video output cannot attach after render shutdown has started.");
        }
        return false;
    }

    if (qmlRoot == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("QML root object is unavailable for video output binding.");
        }
        return false;
    }

    auto* videoItem = qmlRoot->findChild<
        player::playback::infrastructure::mpv::render::MpvVideoItem*>(
        QStringLiteral("mpvVideoItem"),
        Qt::FindChildrenRecursively);
    if (videoItem == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "Player QML root does not contain the required mpvVideoItem output.");
        }
        return false;
    }

    const auto coordinator = videoItem->renderShutdownCoordinator();
    if (coordinator == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "Video output does not expose a render shutdown coordinator.");
        }
        return false;
    }

    videoItem_ = videoItem;
    shutdownCoordinator_ = coordinator;
    bindCoreIfReady();
    return true;
}

void PlayerVideoRenderBinding::beginShutdown() noexcept
{
    if (shutdownStarted_) {
        return;
    }

    shutdownStarted_ = true;
    coreAddress_ = 0;
    if (!videoItem_.isNull()) {
        videoItem_->beginRenderShutdown();
    }
}

bool PlayerVideoRenderBinding::waitForRenderRelease(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (shutdownCoordinator_ == nullptr) {
        videoItem_.clear();
        return true;
    }

    if (!shutdownStarted_) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "Video render release was requested before shutdown began.");
        }
        return false;
    }

    if (!shutdownCoordinator_->waitForRenderRelease(kRenderReleaseTimeout)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "Video render resources did not release within 5 seconds.");
        }
        return false;
    }

    videoItem_.clear();
    shutdownCoordinator_.reset();
    return true;
}

void PlayerVideoRenderBinding::bindCoreIfReady() noexcept
{
    if (shutdownStarted_ || coreAddress_ == 0 || videoItem_.isNull()) {
        return;
    }

    videoItem_->setRenderCoreHandle(
        reinterpret_cast<mpv_handle*>(coreAddress_));
}

} // namespace player::app
