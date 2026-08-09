#include "mpv_video_renderer.h"

namespace player::playback::infrastructure::mpv::render {

void MpvVideoRenderer::synchronize(QQuickFramebufferObject* item)
{
    const auto* videoItem = qobject_cast<MpvVideoItem*>(item);
    if (videoItem == nullptr) {
        presentationState_ = {};
        return;
    }

    presentationState_ = videoItem->presentationState();
}

void MpvVideoRenderer::render()
{
    // Video-frame rendering is introduced by the dedicated renderer task.
}

const MpvVideoPresentationState& MpvVideoRenderer::presentationState() const noexcept
{
    return presentationState_;
}

} // namespace player::playback::infrastructure::mpv::render
