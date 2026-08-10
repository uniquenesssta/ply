#include "render_probe_environment.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSysInfo>

namespace player::tools::render_probe {
namespace {

QString glString(QOpenGLFunctions* functions, GLenum name)
{
    if (functions == nullptr) {
        return {};
    }

    const GLubyte* value = functions->glGetString(name);
    return value != nullptr
        ? QString::fromLatin1(reinterpret_cast<const char*>(value))
        : QString{};
}

} // namespace

RenderProbeEnvironment captureRenderProbeEnvironment(QString* warningMessage)
{
    if (warningMessage != nullptr) {
        warningMessage->clear();
    }

    RenderProbeEnvironment environment;
    environment.osProduct = QSysInfo::prettyProductName();
    environment.kernelType = QSysInfo::kernelType();
    environment.kernelVersion = QSysInfo::kernelVersion();
    environment.currentCpuArchitecture = QSysInfo::currentCpuArchitecture();
    environment.buildCpuArchitecture = QSysInfo::buildCpuArchitecture();
    environment.qtVersion = QString::fromLatin1(qVersion());
#ifdef NDEBUG
    environment.buildType = QStringLiteral("Release");
#else
    environment.buildType = QStringLiteral("Debug");
#endif

    QOpenGLContext context;
    if (!context.create()) {
        if (warningMessage != nullptr) {
            *warningMessage = QStringLiteral("Unable to create the diagnostic OpenGL context; GPU identity fields are unavailable.");
        }
        return environment;
    }

    QOffscreenSurface surface;
    surface.setFormat(context.format());
    surface.create();
    if (!surface.isValid() || !context.makeCurrent(&surface)) {
        if (warningMessage != nullptr) {
            *warningMessage = QStringLiteral("Unable to make the diagnostic OpenGL context current; GPU identity fields are unavailable.");
        }
        return environment;
    }

    QOpenGLFunctions* functions = context.functions();
    if (functions != nullptr) {
        functions->initializeOpenGLFunctions();
    }

    environment.gpuVendor = glString(functions, GL_VENDOR);
    environment.gpuRenderer = glString(functions, GL_RENDERER);
    environment.openGlVersion = glString(functions, GL_VERSION);
    context.doneCurrent();
    return environment;
}

} // namespace player::tools::render_probe
