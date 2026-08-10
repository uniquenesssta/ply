#include "render_pixel_capture_fixture.h"
#include "render_test_fixture.h"
#include "render_video_fixture.h"

#include <QColor>
#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QWindow>
#include <QtTest>

#include <memory>

namespace {

using player::playback::mpv::MpvHandle;
using player::test::render::GeneratedY4mPattern;
using player::test::render::QuickVideoPixelCaptureFixture;
using player::test::render::createInitializedVideoCore;
using player::test::render::loadFileOnWorkerThread;
using player::test::render::setPauseOnWorkerThread;
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

class MpvVideoVisibilityTest final : public QObject
{
    Q_OBJECT

private slots:
    void pausedVideoSurvivesItemHideAndTwentySecondMinimize();
};

void MpvVideoVisibilityTest::pausedVideoSurvivesItemHideAndTwentySecondMinimize()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("visibility-restore.y4m"));
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

    QuickVideoPixelCaptureFixture fixture(QSize(160, 90));
    fixture.videoItem().setRenderCoreHandle(core->nativeHandle());

    const auto visibilityPolicy = fixture.videoItem().renderVisibilityPolicy();
    QVERIFY(visibilityPolicy != nullptr);

    QSignalSpy renderedSpy(&fixture.window(), &QQuickWindow::afterRendering);
    fixture.show();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(visibilityPolicy->snapshot().updatesAllowed, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() >= 1, 3000);

    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);
    QTRY_VERIFY_WITH_TIMEOUT(hasExpectedVerticalOrientation(fixture.grabWindow()), 6000);

    QCOMPARE(setPauseOnWorkerThread(core->nativeHandle(), true), 0);
    QTRY_VERIFY_WITH_TIMEOUT(hasExpectedVerticalOrientation(fixture.grabWindow()), 3000);

    fixture.videoItem().setVisible(false);
    QTRY_VERIFY_WITH_TIMEOUT(!visibilityPolicy->snapshot().updatesAllowed, 1000);
    const int hiddenRenderCount = renderedSpy.count();
    QTest::qWait(1000);
    QVERIFY2(
        renderedSpy.count() <= hiddenRenderCount + 4,
        "Hidden video item continued rendering instead of settling.");

    fixture.videoItem().setVisible(true);
    QTRY_VERIFY_WITH_TIMEOUT(visibilityPolicy->snapshot().updatesAllowed, 1000);
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() > hiddenRenderCount, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(hasExpectedVerticalOrientation(fixture.grabWindow()), 6000);

    fixture.window().showMinimized();
    fixture.window().update();
    QTRY_COMPARE_WITH_TIMEOUT(fixture.window().visibility(), QWindow::Minimized, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(!visibilityPolicy->snapshot().updatesAllowed, 3000);

    const int minimizedRenderCount = renderedSpy.count();
    QTest::qWait(20'000);
    QVERIFY2(
        renderedSpy.count() <= minimizedRenderCount + 4,
        "Minimized video window continued rendering instead of settling.");

    fixture.window().showNormal();
    fixture.window().update();
    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().visibility() != QWindow::Minimized, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 5000);
    QTRY_VERIFY_WITH_TIMEOUT(visibilityPolicy->snapshot().updatesAllowed, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() > minimizedRenderCount, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(hasExpectedVerticalOrientation(fixture.grabWindow()), 6000);

    QCOMPARE(stopPlaybackOnWorkerThread(core->nativeHandle()), 0);

    fixture.videoItem().setRenderCoreHandle(nullptr);
    const int renderCountBeforeDetach = renderedSpy.count();
    fixture.videoItem().update();
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() > renderCountBeforeDetach, 3000);
}

} // namespace

int main(int argc, char* argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication application(argc, argv);
    MpvVideoVisibilityTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "mpv_video_visibility_test.moc"
