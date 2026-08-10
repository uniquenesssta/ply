#include "mpv_video_renderer.h"

#include "mpv_render_context.h"
#include "mpv_render_parameters.h"
#include "mpv_render_update_bridge.h"

#include <mpv/render.h>

#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickOpenGLUtils>
#include <QString>

#include <cstdint>
#include <utility>

namespace player::playback::infrastructure::mpv::render {
namespace {

class OpenGlStateReset final
{
public:
    ~OpenGlStateReset()
    {
        QQuickOpenGLUtils::resetOpenGLState();
    }
};

} // namespace

MpvVideoRenderer::MpvVideoRenderer() = default;

MpvVideoRenderer::~MpvVideoRenderer()
{
    if (!releaseRenderContext()) {
        qCritical("MpvVideoRenderer could not safely release its mpv render context.");
    }
}

void MpvVideoRenderer::synchronize(QQuickFramebufferObject* item)
{
    const auto* videoItem = qobject_cast<MpvVideoItem*>(item);
    if (videoItem == nullptr) {
        presentationState_ = {};
        synchronizedCoreHandle_ = nullptr;
        visibilityPolicy_.reset();
        visibilityRevision_ = 0;
        return;
    }

    presentationState_ = videoItem->presentationState();
    synchronizedCoreHandle_ = videoItem->renderCoreHandle();

    std::shared_ptr<const MpvRenderVisibilityPolicy> visibilityPolicy =
        videoItem->renderVisibilityPolicy();
    if (visibilityPolicy_ != visibilityPolicy) {
        visibilityPolicy_ = std::move(visibilityPolicy);
        visibilityRevision_ = 0;
        framebufferNeedsRender_ = true;
    }
}

QOpenGLFramebufferObject* MpvVideoRenderer::createFramebufferObject(const QSize& size)
{
    framebufferNeedsRender_ = true;
    return new QOpenGLFramebufferObject(size);
}

void MpvVideoRenderer::render()
{
    [[maybe_unused]] OpenGlStateReset stateReset;

    if (!applySynchronizedCoreBinding()) {
        return;
    }

    // Qt owns the decision to execute a render pass. Visibility policy only
    // suppresses libmpv-driven update scheduling in MpvRenderUpdateBridge.
    // Refusing an already scheduled Qt render here would create a second gate
    // and can delay initial render-context creation during show/layout races.
    refreshVisibilityState();

    QOpenGLFramebufferObject* framebuffer = framebufferObject();
    if (framebuffer == nullptr || !framebuffer->isValid()) {
        return;
    }

    if (boundCoreHandle_ == nullptr) {
        clearFramebuffer();
        return;
    }

    if (!ensureRenderContext()) {
        clearFramebuffer();
        return;
    }

    std::uint64_t updateFlags = 0;
    QString errorMessage;
    if (!renderContext_->update(&updateFlags, &errorMessage)) {
        reportErrorOnce(
            updateFailureReported_,
            QStringLiteral("Unable to update the mpv video renderer: %1").arg(errorMessage));
        return;
    }
    updateFailureReported_ = false;

    const bool hasNewFrame = (updateFlags & MPV_RENDER_UPDATE_FRAME) != 0U;
    if (!hasNewFrame && !framebufferNeedsRender_) {
        return;
    }

    MpvRenderParameters renderParameters(MpvOpenGlRenderTarget{
        framebuffer->handle(),
        framebuffer->width(),
        framebuffer->height(),
        0,
        false,
    });
    if (!renderParameters.isValid()) {
        reportErrorOnce(
            renderFailureReported_,
            QStringLiteral("Unable to render video because the Qt Quick framebuffer target is invalid."));
        return;
    }

    if (!renderContext_->render(renderParameters.data(), &errorMessage)) {
        reportErrorOnce(
            renderFailureReported_,
            QStringLiteral("Unable to render the current mpv video frame: %1").arg(errorMessage));
        return;
    }

    renderFailureReported_ = false;
    framebufferNeedsRender_ = false;
}

const MpvVideoPresentationState& MpvVideoRenderer::presentationState() const noexcept
{
    return presentationState_;
}

bool MpvVideoRenderer::applySynchronizedCoreBinding()
{
    if (boundCoreHandle_ == synchronizedCoreHandle_) {
        return true;
    }

    if (renderContext_ != nullptr && !releaseRenderContext()) {
        return false;
    }

    boundCoreHandle_ = synchronizedCoreHandle_;
    renderContextCreationAttempted_ = false;
    updateFailureReported_ = false;
    renderFailureReported_ = false;
    framebufferNeedsRender_ = true;
    return true;
}

void MpvVideoRenderer::refreshVisibilityState()
{
    if (visibilityPolicy_ == nullptr) {
        return;
    }

    const MpvRenderVisibilitySnapshot snapshot = visibilityPolicy_->snapshot();
    if (snapshot.revision == visibilityRevision_) {
        return;
    }

    visibilityRevision_ = snapshot.revision;
    if (snapshot.updatesAllowed) {
        // Restore must repaint even when playback is paused and mpv has no new
        // frame flag. Qt may also have invalidated/recreated the FBO while the
        // window was hidden or minimized.
        framebufferNeedsRender_ = true;
    }
}

bool MpvVideoRenderer::ensureRenderContext()
{
    if (renderContext_ != nullptr) {
        return true;
    }

    if (boundCoreHandle_ == nullptr || renderContextCreationAttempted_) {
        return false;
    }

    renderContextCreationAttempted_ = true;

    QString errorMessage;
    std::unique_ptr<MpvRenderContext> renderContext =
        MpvRenderContext::create(boundCoreHandle_, &errorMessage);
    if (renderContext == nullptr) {
        qWarning().noquote()
            << QStringLiteral("Unable to create the mpv video render context: %1")
                   .arg(errorMessage);
        return false;
    }

    auto updateBridge = std::make_unique<MpvRenderUpdateBridge>(visibilityPolicy_, nullptr);
    QObject::connect(
        updateBridge.get(),
        &MpvRenderUpdateBridge::updateRequested,
        updateBridge.get(),
        [this] {
            update();
        });

    if (!updateBridge->activate(*renderContext, &errorMessage)) {
        qWarning().noquote()
            << QStringLiteral("Unable to activate the mpv video update bridge: %1")
                   .arg(errorMessage);

        QString closeError;
        if (!renderContext->close(&closeError)) {
            qCritical().noquote()
                << QStringLiteral("Unable to release the failed mpv video render context: %1")
                       .arg(closeError);
        }
        return false;
    }

    renderContext_ = std::move(renderContext);
    updateBridge_ = std::move(updateBridge);
    return true;
}

bool MpvVideoRenderer::releaseRenderContext()
{
    if (updateBridge_ != nullptr) {
        QString errorMessage;
        if (!updateBridge_->deactivate(&errorMessage)) {
            qCritical().noquote()
                << QStringLiteral("Unable to deactivate the mpv video update bridge: %1")
                       .arg(errorMessage);
            return false;
        }
        updateBridge_.reset();
    }

    if (renderContext_ != nullptr) {
        QString errorMessage;
        if (!renderContext_->close(&errorMessage)) {
            qCritical().noquote()
                << QStringLiteral("Unable to close the mpv video render context: %1")
                       .arg(errorMessage);
            return false;
        }
        renderContext_.reset();
    }

    boundCoreHandle_ = nullptr;
    renderContextCreationAttempted_ = false;
    return true;
}

void MpvVideoRenderer::clearFramebuffer() noexcept
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context == nullptr) {
        return;
    }

    QOpenGLFunctions* functions = context->functions();
    if (functions == nullptr) {
        return;
    }

    functions->glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
    functions->glClear(GL_COLOR_BUFFER_BIT);
    framebufferNeedsRender_ = true;
}

void MpvVideoRenderer::reportErrorOnce(bool& reported, const QString& message)
{
    if (reported) {
        return;
    }

    reported = true;
    qWarning().noquote() << message;
}

} // namespace player::playback::infrastructure::mpv::render
