#include "app/bootstrap/graphics_backend/graphics_backend_probe.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSurfaceFormat>

namespace player::app {
namespace {

void assignError(QString* errorMessage, const QString& message)
{
    if (errorMessage) {
        *errorMessage = message;
    }
}

QString readOpenGlString(QOpenGLFunctions& functions, GLenum name)
{
    const GLubyte* value = functions.glGetString(name);
    if (!value) {
        return {};
    }

    return QString::fromLatin1(reinterpret_cast<const char*>(value));
}

} // namespace

bool GraphicsBackendProbe::probe(GraphicsBackendInfo& info, QString* errorMessage)
{
    info = {};
    if (errorMessage) {
        errorMessage->clear();
    }

    if (QQuickWindow::graphicsApi() != QSGRendererInterface::OpenGL) {
        assignError(errorMessage, QStringLiteral("Qt Quick graphics API is not OpenGL."));
        return false;
    }

    QOpenGLContext context;
    context.setFormat(QSurfaceFormat::defaultFormat());
    if (!context.create()) {
        assignError(errorMessage, QStringLiteral("Unable to create an OpenGL context."));
        return false;
    }

    QOffscreenSurface surface;
    surface.setFormat(context.format());
    surface.create();
    if (!surface.isValid()) {
        assignError(errorMessage, QStringLiteral("Unable to create the offscreen OpenGL validation surface."));
        return false;
    }

    if (!context.makeCurrent(&surface)) {
        assignError(errorMessage, QStringLiteral("Unable to make the OpenGL validation context current."));
        return false;
    }

    QOpenGLFunctions* functions = context.functions();
    if (!functions) {
        context.doneCurrent();
        assignError(errorMessage, QStringLiteral("OpenGL functions are unavailable for the validation context."));
        return false;
    }

    GraphicsBackendInfo probedInfo;
    probedInfo.vendor = readOpenGlString(*functions, GL_VENDOR);
    probedInfo.renderer = readOpenGlString(*functions, GL_RENDERER);
    probedInfo.version = readOpenGlString(*functions, GL_VERSION);
    probedInfo.shadingLanguageVersion = readOpenGlString(*functions, GL_SHADING_LANGUAGE_VERSION);
    probedInfo.isOpenGles = context.isOpenGLES();

    const QSurfaceFormat actualFormat = context.format();
    probedInfo.majorVersion = actualFormat.majorVersion();
    probedInfo.minorVersion = actualFormat.minorVersion();

    context.doneCurrent();

    if (probedInfo.renderer.isEmpty() || probedInfo.version.isEmpty()) {
        assignError(
            errorMessage,
            QStringLiteral("OpenGL context was created, but GL_RENDERER or GL_VERSION could not be queried."));
        return false;
    }

    info = probedInfo;
    return true;
}

} // namespace player::app
