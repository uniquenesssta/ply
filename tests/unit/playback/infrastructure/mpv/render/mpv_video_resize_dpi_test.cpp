#include "render_quick_fixture.h"
#include "render_test_fixture.h"
#include "render_video_fixture.h"

#include <QColor>
#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QScreen>
#include <QSGRendererInterface>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QWindow>
#include <QtTest>

#include <memory>

namespace {

using player::playback::mpv::MpvHandle;
using player::test::render::GeneratedY4mPattern;
using player::test::render::QuickVideoSurfaceFixture;
using player::test::render::createInitializedVideoCore;
using player::test::render::loadFileOnWorkerThread;
using player::test::render::setPauseOnWorkerThread;
using player::test::render::stopPlaybackOnWorkerThread;
using player::test::render::writeGeneratedY4mVideo;

int pixelLuminance(const QColor& color)
{
    return ((299 * color.red()) + (587 * color.green()) + (114 * color.blue())) / 1000;
}

bool hasUndistortedSquareVideo(const QImage& image)
{
    if (image.isNull() || image.width() < 64 || image.height() < 32) {
        return false;
    }

    const int centerX = image.width() / 2;
    const int topY = image.height() / 4;
    const int bottomY = (image.height() * 3) / 4;
    const int middleY = image.height() / 2;
    const int leftBarX = image.width() / 16;
    const int rightBarX = (image.width() * 15) / 16;

    const int topCenter = pixelLuminance(image.pixelColor(centerX, topY));
    const int bottomCenter = pixelLuminance(image.pixelColor(centerX, bottomY));
    const int leftBar = pixelLuminance(image.pixelColor(leftBarX, middleY));
    const int rightBar = pixelLuminance(image.pixelColor(rightBarX, middleY));

    return topCenter >= 120
        && topCenter >= bottomCenter + 80
        && leftBar <= 32
        && rightBar <= 32;
}

class MpvVideoResizeDpiTest final : public QObject
{
    Q_OBJECT

private slots:
    void framebufferTracksLogicalResizeAtEffectiveDpr();
    void squareVideoKeepsAspectAcrossPausedResize();
    void fullscreenUsesPhysicalFramebufferResolution();
    void differentDprScreenRecreatesPhysicalFramebufferWhenAvailable();
};

void MpvVideoResizeDpiTest::framebufferTracksLogicalResizeAtEffectiveDpr()
{
    QuickVideoSurfaceFixture fixture(QSize(160, 96));
    fixture.show();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        5000);

    auto state = fixture.videoItem().presentationState();
    QCOMPARE(state.logicalSize, QSizeF(160.0, 96.0));
    QCOMPARE(state.devicePixelRatio, fixture.window().effectiveDevicePixelRatio());
    QVERIFY(state.devicePixelRatio > 0.0);

    const QSize initialFramebufferSize = fixture.framebufferSize();

    fixture.resize(QSize(320, 192));

    QTRY_COMPARE_WITH_TIMEOUT(fixture.window().size(), QSize(320, 192), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.videoItem().presentationState().logicalSize == QSizeF(320.0, 192.0),
        3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        5000);

    state = fixture.videoItem().presentationState();
    QCOMPARE(state.devicePixelRatio, fixture.window().effectiveDevicePixelRatio());
    QVERIFY(fixture.framebufferSize() != initialFramebufferSize);
}

void MpvVideoResizeDpiTest::squareVideoKeepsAspectAcrossPausedResize()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("resize-dpi-square.y4m"));
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

    QuickVideoSurfaceFixture fixture(QSize(160, 90));
    fixture.videoItem().setRenderCoreHandle(core->nativeHandle());
    QSignalSpy renderedSpy(&fixture.window(), &QQuickWindow::afterRendering);
    fixture.show();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        5000);

    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);
    QTRY_VERIFY_WITH_TIMEOUT(hasUndistortedSquareVideo(fixture.window().grabWindow()), 6000);

    QCOMPARE(setPauseOnWorkerThread(core->nativeHandle(), true), 0);

    fixture.resize(QSize(320, 180));

    QTRY_COMPARE_WITH_TIMEOUT(fixture.window().size(), QSize(320, 180), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        5000);
    QTRY_VERIFY_WITH_TIMEOUT(hasUndistortedSquareVideo(fixture.window().grabWindow()), 6000);

    QCOMPARE(stopPlaybackOnWorkerThread(core->nativeHandle()), 0);

    fixture.videoItem().setRenderCoreHandle(nullptr);
    const int renderCountBeforeDetach = renderedSpy.count();
    fixture.videoItem().update();
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() > renderCountBeforeDetach, 3000);
}

void MpvVideoResizeDpiTest::fullscreenUsesPhysicalFramebufferResolution()
{
    if (qEnvironmentVariable("PLAYER_R4_06_FULLSCREEN_SMOKE") != QStringLiteral("1")) {
        QSKIP("Fullscreen smoke runs once in the 100% scale test process.");
    }

    QuickVideoSurfaceFixture fixture(QSize(320, 180));
    fixture.show();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        5000);

    fixture.window().showFullScreen();
    fixture.window().update();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().visibility() == QWindow::FullScreen, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.videoItem().presentationState().logicalSize
            == QSizeF(fixture.window().width(), fixture.window().height()),
        5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        8000);

    fixture.window().showNormal();
    fixture.window().update();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().visibility() != QWindow::FullScreen, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        5000);
}

void MpvVideoResizeDpiTest::differentDprScreenRecreatesPhysicalFramebufferWhenAvailable()
{
    if (qEnvironmentVariable("PLAYER_R4_06_MULTISCREEN_SMOKE") != QStringLiteral("1")) {
        QSKIP("Multi-screen DPR smoke runs once in the 100% scale test process.");
    }

    QScreen* sourceScreen = QGuiApplication::primaryScreen();
    if (sourceScreen == nullptr) {
        QSKIP("No primary screen is available for the multi-screen DPR smoke.");
    }

    QScreen* targetScreen = nullptr;
    for (QScreen* screen : QGuiApplication::screens()) {
        if (screen != sourceScreen
            && !qFuzzyCompare(screen->devicePixelRatio(), sourceScreen->devicePixelRatio())) {
            targetScreen = screen;
            break;
        }
    }

    if (targetScreen == nullptr) {
        QSKIP("No second screen with a different DPR is available.");
    }

    QuickVideoSurfaceFixture fixture(QSize(320, 180));
    fixture.window().setScreen(sourceScreen);
    fixture.show();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        5000);

    const qreal sourceDevicePixelRatio = fixture.window().effectiveDevicePixelRatio();
    QCOMPARE(fixture.videoItem().presentationState().devicePixelRatio, sourceDevicePixelRatio);

    fixture.window().setScreen(targetScreen);
    fixture.window().update();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().screen() == targetScreen, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        !qFuzzyCompare(fixture.window().effectiveDevicePixelRatio(), sourceDevicePixelRatio),
        5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        fixture.framebufferSize() == fixture.expectedPhysicalFramebufferSize(),
        8000);
    QCOMPARE(
        fixture.videoItem().presentationState().devicePixelRatio,
        fixture.window().effectiveDevicePixelRatio());
}

} // namespace

int main(int argc, char* argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication application(argc, argv);
    MpvVideoResizeDpiTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "mpv_video_resize_dpi_test.moc"
