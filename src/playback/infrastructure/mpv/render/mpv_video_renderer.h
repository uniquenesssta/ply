#pragma once

#include "mpv_video_item.h"

#include <QQuickFramebufferObject>

namespace player::playback::infrastructure::mpv::render {

class MpvVideoRenderer final : public QQuickFramebufferObject::Renderer
{
public:
    void synchronize(QQuickFramebufferObject* item) override;
    void render() override;

    [[nodiscard]] const MpvVideoPresentationState& presentationState() const noexcept;

private:
    MpvVideoPresentationState presentationState_;
};

} // namespace player::playback::infrastructure::mpv::render
