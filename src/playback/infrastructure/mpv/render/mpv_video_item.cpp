#include "mpv_video_item.h"

#include "mpv_video_renderer.h"

#include <QQuickWindow>

namespace player::playback::infrastructure::mpv::render {

MpvVideoItem::MpvVideoItem(QQuickItem* parent)
    : QQuickFramebufferObject(parent)
{
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

    return MpvVideoPresentationState{
        QSizeF{width(), height()},
        devicePixelRatio,
        isVisible(),
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

} // namespace player::playback::infrastructure::mpv::render
