#include "opengl_proc_resolver.h"

#include <QOpenGLContext>
#include <QSurfaceFormat>

namespace player::playback::infrastructure::mpv::render {
namespace {

void assignError(QString* errorMessage, QString message)
{
    if (errorMessage != nullptr) {
        *errorMessage = std::move(message);
    }
}

bool hasMinimumOpenGlVersion(const QOpenGLContext& context) noexcept
{
    const QSurfaceFormat format = context.format();
    const int majorVersion = format.majorVersion();
    const int minorVersion = format.minorVersion();

    if (context.isOpenGLES()) {
        return majorVersion >= 2;
    }

    return majorVersion > 2 || (majorVersion == 2 && minorVersion >= 1);
}

void* resolveWithContext(QOpenGLContext& context, const char* procedureName) noexcept
{
    if (procedureName == nullptr || procedureName[0] == '\0') {
        return nullptr;
    }

    const QFunctionPointer function = context.getProcAddress(procedureName);
    return reinterpret_cast<void*>(function);
}

} // namespace

bool OpenGlProcResolver::validateCurrentContext(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context == nullptr) {
        assignError(errorMessage, QStringLiteral("No OpenGL context is current on this thread."));
        return false;
    }

    if (!context->isValid()) {
        assignError(errorMessage, QStringLiteral("The current OpenGL context is invalid."));
        return false;
    }

    if (!hasMinimumOpenGlVersion(*context)) {
        const QSurfaceFormat format = context->format();
        assignError(
            errorMessage,
            QStringLiteral("The current OpenGL context does not meet libmpv's minimum OpenGL requirement: desktop OpenGL 2.1 or OpenGL ES 2.0. Current version is %1.%2.")
                .arg(format.majorVersion())
                .arg(format.minorVersion()));
        return false;
    }

    QString lookupError;
    if (resolveCurrent("glGetString", &lookupError) == nullptr) {
        assignError(
            errorMessage,
            QStringLiteral("The current OpenGL context cannot provide the required procedure resolver baseline: %1")
                .arg(lookupError));
        return false;
    }

    return true;
}

void* OpenGlProcResolver::resolveCurrent(
    const char* procedureName,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (procedureName == nullptr || procedureName[0] == '\0') {
        assignError(errorMessage, QStringLiteral("OpenGL procedure name is empty."));
        return nullptr;
    }

    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context == nullptr) {
        assignError(errorMessage, QStringLiteral("No OpenGL context is current on this thread."));
        return nullptr;
    }

    if (!context->isValid()) {
        assignError(errorMessage, QStringLiteral("The current OpenGL context is invalid."));
        return nullptr;
    }

    void* procedure = resolveWithContext(*context, procedureName);
    if (procedure == nullptr) {
        assignError(
            errorMessage,
            QStringLiteral("Required OpenGL procedure '%1' is unavailable from the current context.")
                .arg(QString::fromLatin1(procedureName)));
    }

    return procedure;
}

void* OpenGlProcResolver::resolveForMpv(
    void* expectedContext,
    const char* procedureName) noexcept
{
    if (expectedContext == nullptr || procedureName == nullptr || procedureName[0] == '\0') {
        return nullptr;
    }

    auto* expected = static_cast<QOpenGLContext*>(expectedContext);
    QOpenGLContext* current = QOpenGLContext::currentContext();
    if (current == nullptr || current != expected || !current->isValid()) {
        return nullptr;
    }

    return resolveWithContext(*current, procedureName);
}

} // namespace player::playback::infrastructure::mpv::render
