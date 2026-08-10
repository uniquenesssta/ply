#pragma once

#include <QQuickFramebufferObject>
#include <QSizeF>

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
};

} // namespace player::playback::infrastructure::mpv::render
