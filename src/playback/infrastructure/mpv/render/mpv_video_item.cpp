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
            update();
        });

    connect(
        this,
        &QQuickItem::windowChanged,
        this,
        [this](QQuickWindow* quickWindow) {
            observeWindow(quickWindow);
        });

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
    update();
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

    if (quickWindow != nullptr) {
        windowVisibleConnection_ = connect(
            quickWindow,
            &QWindow::visibleChanged,
            this,
            [this](bool) {
                refreshRenderVisibilityPolicy();
                update();
            });
        windowVisibilityConnection_ = connect(
            quickWindow,
            &QWindow::visibilityChanged,
            this,
            [this](QWindow::Visibility) {
                refreshRenderVisibilityPolicy();
                update();
            });
    }

    refreshRenderVisibilityPolicy();
    update();
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

} // namespace player::playback::infrastructure::mpv::render
