#include "playback/infrastructure/mpv/render/mpv_render_parameters.h"
#include "playback/infrastructure/mpv/render/mpv_video_item.h"
#include "render_test_fixture.h"
#include "render_video_fixture.h"

#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <QColor>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include <memory>

namespace {

using player::playback::infrastructure::mpv::render::MpvOpenGlRenderTarget;
using player::playback::infrastructure::mpv::render::MpvRenderParameters;
using player::playback::infrastructure::mpv::render::MpvVideoItem;
using player::playback::mpv::MpvHandle;
using player::test::render::GeneratedY4mPattern;
using player::test::render::createInitializedVideoCore;
using player::test::render::loadFileOnWorkerThread;
using player::test::render::stopPlaybackOnWorkerThread;
using player::test::render::writeGeneratedY4mVideo;

int pixelLuminance(const QColor& color)
{
    return ((299 * color.red()) + (587 * color.green()) + (114 * color.blue())) / 1000;
}

bool hasExpectedVerticalOrientation(const QImage& image)
{
    if (image.isNull() || image.width() < 4 || image.height() < 4) {
        return false;
    }

    const int centerX = image.width() / 2;
    const QColor top = image.pixelColor(centerX, image.height() / 4);
    const QColor bottom = image.pixelColor(centerX, (image.height() * 3) / 4);
    return pixelLuminance(top) >= pixelLuminance(bottom) + 80;
}

class MpvVideoRendererTest final : public QObject
{
    Q_OBJECT

private slots:
    void renderParametersDescribeQtFramebuffer();
    void generatedVideoRendersIntoQuickFramebufferUpright();
    void generatedVideoContinuesRenderingWithoutExternalWindowWake();
};

void MpvVideoRendererTest::renderParametersDescribeQtFramebuffer()
{
    MpvRenderParameters parameters(MpvOpenGlRenderTarget{
        17U,
        640,
        360,
        0,
        false,
    });

    QVERIFY(parameters.isValid());
    const mpv_render_param* data = parameters.data();
    QCOMPARE(data[0].type, MPV_RENDER_PARAM_OPENGL_FBO);
    QCOMPARE(data[1].type, MPV_RENDER_PARAM_FLIP_Y);
    QCOMPARE(data[2].type, MPV_RENDER_PARAM_INVALID);

    const auto* framebuffer = static_cast<const mpv_opengl_fbo*>(data[0].data);
    QVERIFY(framebuffer != nullptr);
    QCOMPARE(framebuffer->fbo, 17);
    QCOMPARE(framebuffer->w, 640);
    QCOMPARE(framebuffer->h, 360);
    QCOMPARE(framebuffer->internal_format, 0);

    const auto* flipY = static_cast<const int*>(data[1].data);
    QVERIFY(flipY != nullptr);
    QCOMPARE(*flipY, 0);

    MpvRenderParameters invalidParameters(MpvOpenGlRenderTarget{
        0U,
        0,
        360,
        0,
        true,
    });
    QVERIFY(!invalidParameters.isValid());
}

void MpvVideoRendererTest::generatedVideoRendersIntoQuickFramebufferUpright()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("video-renderer-orientation.y4m"));
    QString mediaError;
    QVERIFY2(
        writeGeneratedY4mVideo(
            videoPath,
            120,
            GeneratedY4mPattern::BrightTopDarkBottom,
            &mediaError),
        mediaError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QQuickWindow window;
    window.setColor(Qt::black);
    window.resize(96, 96);

    MpvVideoItem videoItem(window.contentItem());
    videoItem.setWidth(96.0);
    videoItem.setHeight(96.0);
    videoItem.setRenderCoreHandle(core->nativeHandle());

    QSignalSpy renderedSpy(&window, &QQuickWindow::afterRendering);
    window.show();
    window.update();

    QTRY_VERIFY_WITH_TIMEOUT(window.isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() >= 1, 3000);

    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);
    QTRY_VERIFY_WITH_TIMEOUT(hasExpectedVerticalOrientation(window.grabWindow()), 6000);

    QCOMPARE(stopPlaybackOnWorkerThread(core->nativeHandle()), 0);

    videoItem.setRenderCoreHandle(nullptr);
    const int renderCountBeforeDetach = renderedSpy.count();
    videoItem.update();
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() > renderCountBeforeDetach, 3000);

    window.hide();
    window.releaseResources();
    QCoreApplication::processEvents();
}

void MpvVideoRendererTest::generatedVideoContinuesRenderingWithoutExternalWindowWake()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("video-renderer-continuous-wake.y4m"));
    QString mediaError;
    QVERIFY2(
        writeGeneratedY4mVideo(
            videoPath,
            300,
            GeneratedY4mPattern::AnimatedGray,
            &mediaError),
        mediaError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QQuickWindow window;
    window.setColor(Qt::black);
    window.resize(128, 72);

    MpvVideoItem videoItem(window.contentItem());
    videoItem.setWidth(128.0);
    videoItem.setHeight(72.0);
    videoItem.setRenderCoreHandle(core->nativeHandle());

    QSignalSpy renderedSpy(&window, &QQuickWindow::afterRendering);
    window.show();
    window.update();

    QTRY_VERIFY_WITH_TIMEOUT(window.isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() >= 1, 3000);

    const int renderCountBeforeLoad = renderedSpy.count();
    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);

    // No window.update(), resize, grabWindow(), visibility transition or fullscreen
    // transition is allowed here. libmpv redraw notifications alone must wake the
    // GUI-side QQuickFramebufferObject and keep the Scene Graph producing frames.
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() >= renderCountBeforeLoad + 6, 5000);

    const int renderCountAfterStartup = renderedSpy.count();
    QTest::qWait(350);
    QVERIFY(renderedSpy.count() >= renderCountAfterStartup + 3);

    QCOMPARE(stopPlaybackOnWorkerThread(core->nativeHandle()), 0);

    videoItem.setRenderCoreHandle(nullptr);
    const int renderCountBeforeDetach = renderedSpy.count();
    videoItem.update();
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() > renderCountBeforeDetach, 3000);

    window.hide();
    window.releaseResources();
    QCoreApplication::processEvents();
}

} // namespace

int main(int argc, char* argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication application(argc, argv);
    MpvVideoRendererTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "mpv_video_renderer_test.moc"
