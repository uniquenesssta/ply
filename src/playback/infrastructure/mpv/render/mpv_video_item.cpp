#include "mpv_video_item.h"

#include "mpv_video_renderer.h"

#include <QQuickWindow>
#include <QWindow>

namespace player::playback::infrastructure::mpv::render {

MpvVideoItem::MpvVideoItem(QQuickItem* parent)
    : QQuickFramebufferObject(parent)
    , renderVisibilityPolicy_(std::make_shared<MpvRenderVisibilityPolicy>())
{
    connect(
        this,
        &QQuickItem::visibleChanged,
        this,
        [this] {
            refreshRenderVisibilityPolicy();
            scheduleRenderWake();
        });

    connect(
        this,
        &QQuickItem::windowChanged,
        this,
        [this](QQuickWindow* quickWindow) {
            observeWindow(quickWindow);
            scheduleRenderWake();
        });

    // Establish the initial observer/policy state without scheduling a render
    // from inside construction. QML/fixtures may attach the item to a window
    // before the final window/item geometry is known; rendering that transient
    // pre-layout state can create an invalid or stale first FBO lifecycle.
    observeWindow(window());
}

QQuickFramebufferObject::Renderer* MpvVideoItem::createRenderer() const
{
    return new MpvVideoRenderer();
}

MpvVideoPresentationState MpvVideoItem::presentationState() const noexcept
{
    const QQuickWindow* quickWindow = window();
    const qreal devicePixelRatio = quickWindow != nullptr
        ? quickWindow->effectiveDevicePixelRatio()
        : qreal{1.0};
    const bool windowVisible = quickWindow != nullptr && quickWindow->isVisible();
    const bool windowMinimized = quickWindow != nullptr
        && quickWindow->visibility() == QWindow::Minimized;

    return MpvVideoPresentationState{
        QSizeF{width(), height()},
        devicePixelRatio,
        isVisible(),
        windowVisible,
        windowMinimized,
    };
}

void MpvVideoItem::setRenderCoreHandle(mpv_handle* coreHandle) noexcept
{
    if (renderCoreHandle_ == coreHandle) {
        return;
    }

    renderCoreHandle_ = coreHandle;
    scheduleRenderWake();
}

mpv_handle* MpvVideoItem::renderCoreHandle() const noexcept
{
    return renderCoreHandle_;
}

std::shared_ptr<const MpvRenderVisibilityPolicy> MpvVideoItem::renderVisibilityPolicy() const noexcept
{
    return renderVisibilityPolicy_;
}

void MpvVideoItem::observeWindow(QQuickWindow* quickWindow)
{
    QObject::disconnect(windowVisibleConnection_);
    QObject::disconnect(windowVisibilityConnection_);
    QObject::disconnect(windowSceneGraphInitializedConnection_);

    if (quickWindow != nullptr) {
        windowVisibleConnection_ = connect(
            quickWindow,
            &QWindow::visibleChanged,
            this,
            [this](bool) {
                refreshRenderVisibilityPolicy();
                scheduleRenderWake();
            });
        windowVisibilityConnection_ = connect(
            quickWindow,
            &QWindow::visibilityChanged,
            this,
            [this](QWindow::Visibility) {
                refreshRenderVisibilityPolicy();
                scheduleRenderWake();
            });
        windowSceneGraphInitializedConnection_ = connect(
            quickWindow,
            &QQuickWindow::sceneGraphInitialized,
            this,
            [this] {
                refreshRenderVisibilityPolicy();
                scheduleRenderWake();
            },
            Qt::QueuedConnection);
    }

    refreshRenderVisibilityPolicy();
}

void MpvVideoItem::refreshRenderVisibilityPolicy() noexcept
{
    const MpvVideoPresentationState state = presentationState();
    renderVisibilityPolicy_->update(MpvRenderVisibilityInput{
        state.visible,
        state.windowVisible,
        state.windowMinimized,
    });
}

void MpvVideoItem::scheduleRenderWake()
{
    if (renderWakeQueued_
        || !renderVisibilityPolicy_->snapshot().updatesAllowed) {
        return;
    }

    renderWakeQueued_ = true;
    QMetaObject::invokeMethod(
        this,
        [this] {
            renderWakeQueued_ = false;

            QQuickWindow* quickWindow = window();
            if (quickWindow == nullptr
                || !quickWindow->isSceneGraphInitialized()
                || !renderVisibilityPolicy_->snapshot().updatesAllowed) {
                return;
            }

            update();
        },
        Qt::QueuedConnection);
}

} // namespace player::playback::infrastructure::mpv::render
