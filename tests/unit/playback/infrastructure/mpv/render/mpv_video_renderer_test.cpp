#include "playback/infrastructure/mpv/render/mpv_render_parameters.h"
#include "render_pixel_capture_fixture.h"
#include "render_test_fixture.h"
#include "render_video_fixture.h"

#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <QColor>
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
using player::playback::mpv::MpvHandle;
using player::test::render::GeneratedY4mPattern;
using player::test::render::QuickVideoPixelCaptureFixture;
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
            900,
            GeneratedY4mPattern::BrightTopDarkBottom,
            &mediaError),
        mediaError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QuickVideoPixelCaptureFixture fixture(QSize(96, 96));
    fixture.videoItem().setRenderCoreHandle(core->nativeHandle());

    QSignalSpy renderedSpy(&fixture.window(), &QQuickWindow::afterRendering);
    fixture.create();

    const QImage initialFrame = fixture.grabWindow();
    QVERIFY(!initialFrame.isNull());
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() >= 1, 3000);

    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);
    QTRY_VERIFY_WITH_TIMEOUT(hasExpectedVerticalOrientation(fixture.grabWindow()), 6000);

    QCOMPARE(stopPlaybackOnWorkerThread(core->nativeHandle()), 0);

    fixture.videoItem().setRenderCoreHandle(nullptr);
    const int renderCountBeforeDetach = renderedSpy.count();
    fixture.videoItem().update();
    QVERIFY(!fixture.grabWindow().isNull());
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() > renderCountBeforeDetach, 3000);
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
