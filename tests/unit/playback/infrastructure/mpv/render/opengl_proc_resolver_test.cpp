#include "playback/infrastructure/mpv/render/opengl_proc_resolver.h"

#include <mpv/render_gl.h>

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QtTest>

namespace {

using player::playback::infrastructure::mpv::render::OpenGlProcResolver;

class CurrentOffscreenContext final {
public:
    bool makeCurrent(QString* errorMessage)
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }

        context_.setFormat(QSurfaceFormat::defaultFormat());
        if (!context_.create()) {
            assignError(errorMessage, QStringLiteral("Unable to create the OpenGL test context."));
            return false;
        }

        surface_.setFormat(context_.format());
        surface_.create();
        if (!surface_.isValid()) {
            assignError(errorMessage, QStringLiteral("Unable to create the offscreen OpenGL test surface."));
            return false;
        }

        if (!context_.makeCurrent(&surface_)) {
            assignError(errorMessage, QStringLiteral("Unable to make the OpenGL test context current."));
            return false;
        }

        return true;
    }

    ~CurrentOffscreenContext()
    {
        if (QOpenGLContext::currentContext() == &context_) {
            context_.doneCurrent();
        }
    }

    QOpenGLContext& context() noexcept
    {
        return context_;
    }

private:
    static void assignError(QString* errorMessage, const QString& message)
    {
        if (errorMessage != nullptr) {
            *errorMessage = message;
        }
    }

    QOpenGLContext context_;
    QOffscreenSurface surface_;
};

class OpenGlProcResolverTest final : public QObject {
    Q_OBJECT

private slots:
    void rejectsMissingCurrentContext();
    void validatesCurrentOffscreenContext();
    void resolvesKnownProcedureAndReportsMissingProcedure();
    void mpvCallbackRequiresExpectedCurrentContext();
};

void OpenGlProcResolverTest::rejectsMissingCurrentContext()
{
    QVERIFY(QOpenGLContext::currentContext() == nullptr);

    QString errorMessage;
    QVERIFY(!OpenGlProcResolver::validateCurrentContext(&errorMessage));
    QVERIFY(!errorMessage.isEmpty());

    errorMessage.clear();
    QVERIFY(OpenGlProcResolver::resolveCurrent("glGetString", &errorMessage) == nullptr);
    QVERIFY(!errorMessage.isEmpty());
}

void OpenGlProcResolverTest::validatesCurrentOffscreenContext()
{
    CurrentOffscreenContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.makeCurrent(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString errorMessage;
    QVERIFY2(
        OpenGlProcResolver::validateCurrentContext(&errorMessage),
        errorMessage.toLocal8Bit().constData());
    QVERIFY(errorMessage.isEmpty());
}

void OpenGlProcResolverTest::resolvesKnownProcedureAndReportsMissingProcedure()
{
    CurrentOffscreenContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.makeCurrent(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString errorMessage;
    QVERIFY(OpenGlProcResolver::resolveCurrent("glGetString", &errorMessage) != nullptr);
    QVERIFY(errorMessage.isEmpty());

    errorMessage.clear();
    QVERIFY(OpenGlProcResolver::resolveCurrent("", &errorMessage) == nullptr);
    QVERIFY(!errorMessage.isEmpty());

    errorMessage.clear();
    QVERIFY(
        OpenGlProcResolver::resolveCurrent(
            "glPlayerProcedureThatMustNotExist_8B98CBBD",
            &errorMessage)
        == nullptr);
    QVERIFY(!errorMessage.isEmpty());
}

void OpenGlProcResolverTest::mpvCallbackRequiresExpectedCurrentContext()
{
    CurrentOffscreenContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.makeCurrent(&fixtureError), fixtureError.toLocal8Bit().constData());

    mpv_opengl_init_params initParams{};
    initParams.get_proc_address = &OpenGlProcResolver::resolveForMpv;
    initParams.get_proc_address_ctx = &fixture.context();

    QVERIFY(initParams.get_proc_address(initParams.get_proc_address_ctx, "glGetString") != nullptr);
    QVERIFY(initParams.get_proc_address(initParams.get_proc_address_ctx, "") == nullptr);
    QVERIFY(initParams.get_proc_address(nullptr, "glGetString") == nullptr);

    fixture.context().doneCurrent();
    QVERIFY(initParams.get_proc_address(initParams.get_proc_address_ctx, "glGetString") == nullptr);
}

} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    OpenGlProcResolverTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "opengl_proc_resolver_test.moc"
