#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/render/mpv_render_context.h"

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QtTest>

#include <cstdint>
#include <memory>

namespace {

using player::playback::infrastructure::mpv::render::MpvRenderContext;
using player::playback::mpv::MpvHandle;
using player::playback::mpv::MpvInitializer;

class OffscreenContext final {
public:
    bool initialize(QString* errorMessage)
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

        return makeCurrent(errorMessage);
    }

    bool makeCurrent(QString* errorMessage = nullptr)
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }

        if (!context_.makeCurrent(&surface_)) {
            assignError(errorMessage, QStringLiteral("Unable to make the OpenGL test context current."));
            return false;
        }

        return true;
    }

    void doneCurrent() noexcept
    {
        if (QOpenGLContext::currentContext() == &context_) {
            context_.doneCurrent();
        }
    }

    ~OffscreenContext()
    {
        doneCurrent();
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

std::unique_ptr<MpvHandle> createInitializedCore(QString* errorMessage)
{
    std::unique_ptr<MpvHandle> handle = MpvHandle::create(errorMessage);
    if (!handle) {
        return {};
    }

    if (!MpvInitializer::initializeProduct(*handle, errorMessage)) {
        return {};
    }

    return handle;
}

class MpvRenderContextTest final : public QObject {
    Q_OBJECT

private slots:
    void rejectsMissingCurrentContext();
    void createUpdateRenderFreeLoop();
    void rejectsWrongCurrentContext();
    void libmpvRejectsUnknownApiType();
};

void MpvRenderContextTest::rejectsMissingCurrentContext()
{
    QVERIFY(QOpenGLContext::currentContext() == nullptr);

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QString errorMessage;
    std::unique_ptr<MpvRenderContext> context =
        MpvRenderContext::create(core->nativeHandle(), &errorMessage);

    QVERIFY(context == nullptr);
    QVERIFY(!errorMessage.isEmpty());
}

void MpvRenderContextTest::createUpdateRenderFreeLoop()
{
    OffscreenContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    mpv_opengl_fbo target{0, 16, 16, 0};
    int flipY = 0;
    mpv_render_param renderParams[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &target},
        {MPV_RENDER_PARAM_FLIP_Y, &flipY},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    for (int iteration = 0; iteration < 3; ++iteration) {
        QString errorMessage;
        std::unique_ptr<MpvRenderContext> context =
            MpvRenderContext::create(core->nativeHandle(), &errorMessage);
        QVERIFY2(context != nullptr, errorMessage.toLocal8Bit().constData());
        QVERIFY(context->isOpen());

        std::uint64_t updateFlags = 0;
        QVERIFY2(
            context->update(&updateFlags, &errorMessage),
            errorMessage.toLocal8Bit().constData());

        QVERIFY2(
            context->render(renderParams, &errorMessage),
            errorMessage.toLocal8Bit().constData());

        QVERIFY2(context->close(&errorMessage), errorMessage.toLocal8Bit().constData());
        QVERIFY(!context->isOpen());
        QVERIFY2(context->close(&errorMessage), errorMessage.toLocal8Bit().constData());
    }
}

void MpvRenderContextTest::rejectsWrongCurrentContext()
{
    OffscreenContext ownerContext;
    QString fixtureError;
    QVERIFY2(ownerContext.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QString errorMessage;
    std::unique_ptr<MpvRenderContext> context =
        MpvRenderContext::create(core->nativeHandle(), &errorMessage);
    QVERIFY2(context != nullptr, errorMessage.toLocal8Bit().constData());

    ownerContext.doneCurrent();

    OffscreenContext otherContext;
    QVERIFY2(otherContext.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    std::uint64_t updateFlags = 0;
    QVERIFY(!context->update(&updateFlags, &errorMessage));
    QVERIFY(!errorMessage.isEmpty());
    QVERIFY(context->isOpen());

    QVERIFY(!context->close(&errorMessage));
    QVERIFY(!errorMessage.isEmpty());
    QVERIFY(context->isOpen());

    otherContext.doneCurrent();
    QVERIFY2(ownerContext.makeCurrent(&fixtureError), fixtureError.toLocal8Bit().constData());
    QVERIFY2(context->close(&errorMessage), errorMessage.toLocal8Bit().constData());
}

void MpvRenderContextTest::libmpvRejectsUnknownApiType()
{
    OffscreenContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    char invalidApiType[] = "player-unknown-render-api";
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, invalidApiType},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    mpv_render_context* rawContext = nullptr;
    const int result = mpv_render_context_create(&rawContext, core->nativeHandle(), params);

    QCOMPARE(result, MPV_ERROR_NOT_IMPLEMENTED);
    QVERIFY(rawContext == nullptr);
}

} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    MpvRenderContextTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "mpv_render_context_test.moc"
