#pragma once

#include "mpv_video_item.h"

#include <QQuickFramebufferObject>

#include <cstdint>
#include <memory>

class QOpenGLFramebufferObject;
class QString;
struct mpv_handle;

namespace player::playback::infrastructure::mpv::render {

class MpvRenderContext;
class MpvRenderShutdownCoordinator;
class MpvRenderUpdateBridge;

class MpvVideoRenderer final : public QQuickFramebufferObject::Renderer
{
public:
    MpvVideoRenderer();
    explicit MpvVideoRenderer(
        std::shared_ptr<MpvRenderShutdownCoordinator> shutdownCoordinator);
    ~MpvVideoRenderer() override;

    void synchronize(QQuickFramebufferObject* item) override;
    [[nodiscard]] QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override;
    void render() override;

    [[nodiscard]] const MpvVideoPresentationState& presentationState() const noexcept;

private:
    [[nodiscard]] bool applySynchronizedCoreBinding();
    [[nodiscard]] bool renderUpdatesAllowed();
    [[nodiscard]] bool ensureRenderContext();
    [[nodiscard]] bool releaseRenderContext();
    void clearFramebuffer() noexcept;
    static void reportErrorOnce(bool& reported, const QString& message);

    MpvVideoPresentationState presentationState_;
    mpv_handle* synchronizedCoreHandle_ = nullptr;
    mpv_handle* boundCoreHandle_ = nullptr;
    std::shared_ptr<const MpvRenderVisibilityPolicy> visibilityPolicy_;
    std::shared_ptr<MpvRenderShutdownCoordinator> shutdownCoordinator_;
    std::uint64_t visibilityRevision_ = 0;
    std::unique_ptr<MpvRenderContext> renderContext_;
    std::unique_ptr<MpvRenderUpdateBridge> updateBridge_;
    bool renderContextCreationAttempted_ = false;
    bool framebufferNeedsRender_ = true;
    bool updateFailureReported_ = false;
    bool renderFailureReported_ = false;
};

} // namespace player::playback::infrastructure::mpv::render
