#pragma once

#include "mpv_render_visibility_policy.h"

#include <QMetaObject>
#include <QQuickFramebufferObject>
#include <QSizeF>

#include <memory>

class QQuickWindow;
struct mpv_handle;

namespace player::playback::infrastructure::mpv::render {

struct MpvVideoPresentationState final
{
    QSizeF logicalSize;
    qreal devicePixelRatio = 1.0;
    bool visible = false;
    bool windowVisible = false;
    bool windowMinimized = false;

    bool operator==(const MpvVideoPresentationState&) const = default;
};

class MpvVideoItem : public QQuickFramebufferObject
{
    Q_OBJECT

public:
    explicit MpvVideoItem(QQuickItem* parent = nullptr);

    [[nodiscard]] Renderer* createRenderer() const override;
    [[nodiscard]] MpvVideoPresentationState presentationState() const noexcept;

    void setRenderCoreHandle(mpv_handle* coreHandle) noexcept;
    [[nodiscard]] mpv_handle* renderCoreHandle() const noexcept;
    [[nodiscard]] std::shared_ptr<const MpvRenderVisibilityPolicy> renderVisibilityPolicy() const noexcept;

private:
    void observeWindow(QQuickWindow* quickWindow);
    void refreshRenderVisibilityPolicy() noexcept;

    mpv_handle* renderCoreHandle_ = nullptr;
    std::shared_ptr<MpvRenderVisibilityPolicy> renderVisibilityPolicy_;
    QMetaObject::Connection windowVisibleConnection_;
    QMetaObject::Connection windowVisibilityConnection_;
};

} // namespace player::playback::infrastructure::mpv::render
