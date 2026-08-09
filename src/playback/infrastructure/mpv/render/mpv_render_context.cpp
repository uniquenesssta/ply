#include "mpv_render_context.h"

#include "opengl_proc_resolver.h"

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <QDebug>
#include <QOpenGLContext>
#include <QString>

#include <thread>
#include <utility>

namespace player::playback::infrastructure::mpv::render {
namespace {

void assignError(QString* errorMessage, QString message)
{
    if (errorMessage != nullptr) {
        *errorMessage = std::move(message);
    }
}

QString renderApiErrorMessage(const char* operation, int result)
{
    return QStringLiteral("%1 failed: %2 (%3).")
        .arg(
            QString::fromLatin1(operation),
            QString::fromUtf8(mpv_error_string(result)))
        .arg(result);
}

} // namespace

std::unique_ptr<MpvRenderContext> MpvRenderContext::create(
    mpv_handle* coreHandle,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (coreHandle == nullptr) {
        assignError(
            errorMessage,
            QStringLiteral("Cannot create an mpv render context without an mpv core."));
        return {};
    }

    QString contextError;
    if (!OpenGlProcResolver::validateCurrentContext(&contextError)) {
        assignError(
            errorMessage,
            QStringLiteral("Cannot create an mpv render context: %1").arg(contextError));
        return {};
    }

    QOpenGLContext* openGlContext = QOpenGLContext::currentContext();

    mpv_opengl_init_params openGlInitParams{};
    openGlInitParams.get_proc_address = &OpenGlProcResolver::resolveForMpv;
    openGlInitParams.get_proc_address_ctx = openGlContext;

    mpv_render_param params[] = {
        {
            MPV_RENDER_PARAM_API_TYPE,
            const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL),
        },
        {
            MPV_RENDER_PARAM_OPENGL_INIT_PARAMS,
            &openGlInitParams,
        },
        {
            MPV_RENDER_PARAM_INVALID,
            nullptr,
        },
    };

    mpv_render_context* context = nullptr;
    const int result = mpv_render_context_create(&context, coreHandle, params);
    if (result < 0) {
        assignError(errorMessage, renderApiErrorMessage("mpv_render_context_create", result));
        return {};
    }

    if (context == nullptr) {
        assignError(
            errorMessage,
            QStringLiteral("mpv_render_context_create succeeded without returning a context."));
        return {};
    }

    return std::unique_ptr<MpvRenderContext>(
        new MpvRenderContext(context, openGlContext, std::this_thread::get_id()));
}

MpvRenderContext::MpvRenderContext(
    mpv_render_context* context,
    QOpenGLContext* openGlContext,
    std::thread::id ownerThreadId)
    : context_(context)
    , openGlContext_(openGlContext)
    , ownerThreadId_(ownerThreadId)
{
}

MpvRenderContext::~MpvRenderContext()
{
    if (context_ == nullptr) {
        return;
    }

    QString errorMessage;
    if (!close(&errorMessage)) {
        qCritical().noquote()
            << QStringLiteral("MpvRenderContext destruction could not safely free libmpv state: %1")
                   .arg(errorMessage);
    }
}

bool MpvRenderContext::isOpen() const noexcept
{
    return context_ != nullptr;
}

bool MpvRenderContext::close(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (context_ == nullptr) {
        return true;
    }

    if (!validateRenderAccess(errorMessage)) {
        return false;
    }

    mpv_render_context* context = std::exchange(context_, nullptr);
    mpv_render_context_free(context);
    return true;
}

bool MpvRenderContext::update(
    std::uint64_t* updateFlags,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (updateFlags == nullptr) {
        assignError(errorMessage, QStringLiteral("Render update requires an output flag pointer."));
        return false;
    }

    *updateFlags = 0;

    if (!validateRenderAccess(errorMessage)) {
        return false;
    }

    *updateFlags = mpv_render_context_update(context_);
    return true;
}

bool MpvRenderContext::render(
    mpv_render_param* params,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (params == nullptr) {
        assignError(errorMessage, QStringLiteral("Render parameters cannot be null."));
        return false;
    }

    if (!validateRenderAccess(errorMessage)) {
        return false;
    }

    const int result = mpv_render_context_render(context_, params);
    if (result < 0) {
        assignError(errorMessage, renderApiErrorMessage("mpv_render_context_render", result));
        return false;
    }

    return true;
}

bool MpvRenderContext::validateRenderAccess(QString* errorMessage) const
{
    if (context_ == nullptr) {
        assignError(errorMessage, QStringLiteral("The mpv render context is closed."));
        return false;
    }

    if (std::this_thread::get_id() != ownerThreadId_) {
        assignError(
            errorMessage,
            QStringLiteral("The mpv render context can only be used on its owner render thread."));
        return false;
    }

    QOpenGLContext* expectedContext = openGlContext_.data();
    if (expectedContext == nullptr) {
        assignError(
            errorMessage,
            QStringLiteral("The OpenGL context used to create the mpv render context no longer exists."));
        return false;
    }

    QOpenGLContext* currentContext = QOpenGLContext::currentContext();
    if (currentContext != expectedContext) {
        assignError(
            errorMessage,
            QStringLiteral("The OpenGL context used to create the mpv render context is not current."));
        return false;
    }

    if (!currentContext->isValid()) {
        assignError(errorMessage, QStringLiteral("The current OpenGL context is invalid."));
        return false;
    }

    return true;
}

} // namespace player::playback::infrastructure::mpv::render
