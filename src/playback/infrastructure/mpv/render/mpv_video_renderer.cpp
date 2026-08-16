#include "mpv_video_renderer.h"

#include "mpv_render_context.h"
#include "mpv_render_parameters.h"
#include "mpv_render_shutdown_coordinator.h"
#include "mpv_render_update_bridge.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickOpenGLUtils>
#include <QString>

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

MpvVideoRenderer::MpvVideoRenderer(
    std::shared_ptr<MpvRenderShutdownCoordinator> shutdownCoordinator)
    : shutdownCoordinator_(std::move(shutdownCoordinator))
{
}

MpvVideoRenderer::~MpvVideoRenderer()
{
    if (!releaseRenderContext()) {
        qCritical("MpvVideoRenderer could not safely release its mpv render context.");
    }
}

void MpvVideoRenderer::synchronize(QQuickFramebufferObject* item)
{
    auto* videoItem = qobject_cast<MpvVideoItem*>(item);
    if (videoItem == nullptr) {
        presentationState_ = {};
        synchronizedCoreHandle_ = nullptr;
        visibilityPolicy_.reset();
        return;
    }

    presentationState_ = videoItem->presentationState();
    synchronizedCoreHandle_ = videoItem->renderCoreHandle();

    std::shared_ptr<const MpvRenderVisibilityPolicy> visibilityPolicy =
        videoItem->renderVisibilityPolicy();
    if (visibilityPolicy_ != visibilityPolicy) {
        visibilityPolicy_ = std::move(visibilityPolicy);
    }

    if (updateBridge_ == nullptr) {
        updateBridge_ = std::make_unique<MpvRenderUpdateBridge>(
            visibilityPolicy_,
            shutdownCoordinator_,
            nullptr);

        const std::shared_ptr<const MpvRenderUpdateDeliveryGate> deliveryGate =
            updateBridge_->deliveryGate();
        const std::shared_ptr<const MpvRenderVisibilityPolicy> wakeVisibilityPolicy =
            visibilityPolicy_;
        const std::shared_ptr<MpvRenderShutdownCoordinator> wakeShutdownCoordinator =
            shutdownCoordinator_;

        QObject::connect(
            updateBridge_.get(),
            &MpvRenderUpdateBridge::updateRequested,
            videoItem,
            [videoItem,
             deliveryGate,
             wakeVisibilityPolicy,
             wakeShutdownCoordinator](quint64 activationEpoch) {
                if (deliveryGate == nullptr || !deliveryGate->allows(activationEpoch)) {
                    return;
                }
                if (wakeShutdownCoordinator != nullptr
                    && wakeShutdownCoordinator->isShutdownRequested()) {
                    return;
                }
                if (wakeVisibilityPolicy != nullptr
                    && !wakeVisibilityPolicy->snapshot().updatesAllowed) {
                    return;
                }

                // libmpv may invoke its redraw callback from a non-GUI thread.
                // The queued receiver is the QQuickFramebufferObject item, so the
                // scene graph is woken from the GUI side instead of depending on
                // the render thread's own event queue to wake itself.
                videoItem->update();
            },
            Qt::QueuedConnection);
    }
}

QOpenGLFramebufferObject* MpvVideoRenderer::createFramebufferObject(const QSize& size)
{
    return new QOpenGLFramebufferObject(size);
}

void MpvVideoRenderer::render()
{
    [[maybe_unused]] OpenGlStateReset stateReset;

    if (shutdownCoordinator_ != nullptr
        && shutdownCoordinator_->isShutdownRequested()) {
        (void)releaseRenderContext();
        return;
    }

    MpvRenderShutdownCoordinator::RenderSection renderSection;
    if (shutdownCoordinator_ != nullptr) {
        renderSection = shutdownCoordinator_->tryEnterRenderSection();
        if (!renderSection) {
            (void)releaseRenderContext();
            return;
        }
    }

    if (!applySynchronizedCoreBinding()) {
        return;
    }

    // Do not create or use the libmpv render context while the item/window is
    // hidden or minimized. Visibility transitions schedule a separate queued Qt
    // render wake from MpvVideoItem once the GUI state has settled.
    if (!renderUpdatesAllowed()) {
        return;
    }

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

    // This renderer uses libmpv's default callback-driven control mode. Without
    // MPV_RENDER_PARAM_ADVANCED_CONTROL, mpv_render_context_update() is optional
    // and its FRAME flag must not be used as a render gate. A Qt render pass can
    // therefore always redraw the current mpv frame; libmpv redraw callbacks are
    // responsible for scheduling future passes through MpvRenderUpdateBridge.
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

    QString errorMessage;
    if (!renderContext_->render(renderParameters.data(), &errorMessage)) {
        reportErrorOnce(
            renderFailureReported_,
            QStringLiteral("Unable to render the current mpv video frame: %1").arg(errorMessage));
        return;
    }

    renderFailureReported_ = false;
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
    renderFailureReported_ = false;
    return true;
}

bool MpvVideoRenderer::renderUpdatesAllowed() const noexcept
{
    if (visibilityPolicy_ == nullptr) {
        return presentationState_.visible;
    }

    return visibilityPolicy_->snapshot().updatesAllowed;
}

bool MpvVideoRenderer::ensureRenderContext()
{
    if (renderContext_ != nullptr) {
        return true;
    }

    if (boundCoreHandle_ == nullptr || renderContextCreationAttempted_) {
        return false;
    }

    if (shutdownCoordinator_ != nullptr
        && shutdownCoordinator_->isShutdownRequested()) {
        return false;
    }

    if (updateBridge_ == nullptr) {
        qWarning("Unable to create the mpv video render context because the GUI wake bridge is unavailable.");
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

    if (shutdownCoordinator_ != nullptr) {
        shutdownCoordinator_->noteRenderContextCreated();
    }

    if (!updateBridge_->activate(*renderContext, &errorMessage)) {
        qWarning().noquote()
            << QStringLiteral("Unable to activate the mpv video update bridge: %1")
                   .arg(errorMessage);

        QString closeError;
        if (!renderContext->close(&closeError)) {
            qCritical().noquote()
                << QStringLiteral("Unable to release the failed mpv video render context: %1")
                       .arg(closeError);
            return false;
        }

        if (shutdownCoordinator_ != nullptr) {
            shutdownCoordinator_->noteRenderContextReleased();
        }
        return false;
    }

    renderContext_ = std::move(renderContext);
    qInfo("mpv video render context active");
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

        if (shutdownCoordinator_ != nullptr) {
            shutdownCoordinator_->noteRenderContextReleased();
        }
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
