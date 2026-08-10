#pragma once

#include <QQuickFramebufferObject>
#include <QSizeF>

struct mpv_handle;

namespace player::playback::infrastructure::mpv::render {

struct MpvVideoPresentationState final
{
    QSizeF logicalSize;
    qreal devicePixelRatio = 1.0;
    bool visible = false;

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

private:
    mpv_handle* renderCoreHandle_ = nullptr;
};

} // namespace player::playback::infrastructure::mpv::render
