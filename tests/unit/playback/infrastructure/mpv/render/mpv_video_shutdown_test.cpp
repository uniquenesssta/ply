#include "playback/infrastructure/mpv/render/mpv_render_shutdown_coordinator.h"
#include "render_pixel_capture_fixture.h"
#include "render_test_fixture.h"
#include "render_video_fixture.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QWindow>
#include <QtTest>

#include <chrono>
#include <memory>

namespace {

using player::playback::infrastructure::mpv::render::MpvRenderShutdownCoordinator;
using player::playback::mpv::MpvHandle;
using player::test::render::GeneratedY4mPattern;
using player::test::render::QuickVideoPixelCaptureFixture;
using player::test::render::createInitializedVideoCore;
using player::test::render::loadFileOnWorkerThread;
using player::test::render::writeGeneratedY4mVideo;
using namespace std::chrono_literals;

class MpvVideoShutdownTest final : public QObject
{
    Q_OBJECT

private slots:
    void playingCloseReleasesRenderBeforeCoreDestroy();
    void resizeCloseReleasesRenderBeforeCoreDestroy();
    void minimizedCloseReleasesRenderBeforeCoreDestroy();
    void repeatedRendererLifecycleTwelveCycles();
};

void MpvVideoShutdownTest::playingCloseReleasesRenderBeforeCoreDestroy()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("shutdown-playing.y4m"));
    QString error;
    QVERIFY2(
        writeGeneratedY4mVideo(
            videoPath,
            300,
            GeneratedY4mPattern::AnimatedGray,
            &error),
        qPrintable(error));

    std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&error);
    QVERIFY2(core != nullptr, qPrintable(error));

    QuickVideoPixelCaptureFixture fixture(QSize(160, 90));
    const auto coordinator = fixture.videoItem().renderShutdownCoordinator();
    QVERIFY(coordinator != nullptr);

    fixture.videoItem().setRenderCoreHandle(core->nativeHandle());
    QSignalSpy renderedSpy(&fixture.window(), &QQuickWindow::afterRendering);
    fixture.show();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
    QTRY_COMPARE_WITH_TIMEOUT(coordinator->snapshot().liveRenderContexts, std::size_t{1}, 3000);

    const int renderCountBeforeLoad = renderedSpy.count();
    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);
    QTRY_VERIFY_WITH_TIMEOUT(renderedSpy.count() > renderCountBeforeLoad, 5000);

    fixture.videoItem().beginRenderShutdown();
    QTRY_VERIFY_WITH_TIMEOUT(coordinator->snapshot().renderReleased, 5000);
    QVERIFY(coordinator->waitForRenderRelease(0ms));
    QCOMPARE(coordinator->snapshot().liveRenderContexts, std::size_t{0});

    core->close();
    QVERIFY(!core->isOpen());

    QTest::qWait(50);
    QVERIFY(coordinator->snapshot().renderReleased);
}

void MpvVideoShutdownTest::resizeCloseReleasesRenderBeforeCoreDestroy()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("shutdown-resize.y4m"));
    QString error;
    QVERIFY2(
        writeGeneratedY4mVideo(
            videoPath,
            300,
            GeneratedY4mPattern::AnimatedGray,
            &error),
        qPrintable(error));

    std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&error);
    QVERIFY2(core != nullptr, qPrintable(error));

    QuickVideoPixelCaptureFixture fixture(QSize(160, 90));
    const auto coordinator = fixture.videoItem().renderShutdownCoordinator();
    QVERIFY(coordinator != nullptr);

    fixture.videoItem().setRenderCoreHandle(core->nativeHandle());
    fixture.show();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
    QTRY_COMPARE_WITH_TIMEOUT(coordinator->snapshot().liveRenderContexts, std::size_t{1}, 3000);
    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);

    fixture.resize(QSize(200, 112));
    fixture.resize(QSize(240, 135));
    fixture.resize(QSize(176, 99));
    fixture.videoItem().beginRenderShutdown();

    QTRY_VERIFY_WITH_TIMEOUT(coordinator->snapshot().renderReleased, 5000);
    QVERIFY(coordinator->waitForRenderRelease(0ms));
    QCOMPARE(coordinator->snapshot().activeRenderSections, std::size_t{0});
    QCOMPARE(coordinator->snapshot().liveRenderContexts, std::size_t{0});

    core->close();
    QVERIFY(!core->isOpen());
}

void MpvVideoShutdownTest::minimizedCloseReleasesRenderBeforeCoreDestroy()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("shutdown-minimized.y4m"));
    QString error;
    QVERIFY2(
        writeGeneratedY4mVideo(
            videoPath,
            300,
            GeneratedY4mPattern::AnimatedGray,
            &error),
        qPrintable(error));

    std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&error);
    QVERIFY2(core != nullptr, qPrintable(error));

    QuickVideoPixelCaptureFixture fixture(QSize(160, 90));
    const auto coordinator = fixture.videoItem().renderShutdownCoordinator();
    QVERIFY(coordinator != nullptr);

    fixture.videoItem().setRenderCoreHandle(core->nativeHandle());
    fixture.show();

    QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
    QTRY_COMPARE_WITH_TIMEOUT(coordinator->snapshot().liveRenderContexts, std::size_t{1}, 3000);
    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);

    fixture.window().showMinimized();
    fixture.window().update();
    QTRY_COMPARE_WITH_TIMEOUT(fixture.window().visibility(), QWindow::Minimized, 5000);

    fixture.videoItem().beginRenderShutdown();
    QTRY_VERIFY_WITH_TIMEOUT(coordinator->snapshot().renderReleased, 5000);
    QVERIFY(coordinator->waitForRenderRelease(0ms));
    QCOMPARE(coordinator->snapshot().liveRenderContexts, std::size_t{0});

    core->close();
    QVERIFY(!core->isOpen());

    QCoreApplication::processEvents();
}

void MpvVideoShutdownTest::repeatedRendererLifecycleTwelveCycles()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("shutdown-cycles.y4m"));
    QString error;
    QVERIFY2(
        writeGeneratedY4mVideo(
            videoPath,
            120,
            GeneratedY4mPattern::AnimatedGray,
            &error),
        qPrintable(error));

    for (int cycle = 0; cycle < 12; ++cycle) {
        std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&error);
        QVERIFY2(
            core != nullptr,
            qPrintable(QStringLiteral("cycle %1 core init failed: %2").arg(cycle).arg(error)));

        QuickVideoPixelCaptureFixture fixture(QSize(128 + (cycle % 3) * 16, 72 + (cycle % 3) * 9));
        const auto coordinator = fixture.videoItem().renderShutdownCoordinator();
        QVERIFY(coordinator != nullptr);

        fixture.videoItem().setRenderCoreHandle(core->nativeHandle());
        fixture.show();

        QTRY_VERIFY_WITH_TIMEOUT(fixture.window().isExposed(), 3000);
        QTRY_COMPARE_WITH_TIMEOUT(coordinator->snapshot().liveRenderContexts, std::size_t{1}, 3000);
        QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);

        if ((cycle % 3) == 1) {
            fixture.resize(QSize(192, 108));
        } else if ((cycle % 3) == 2) {
            fixture.window().showMinimized();
            fixture.window().update();
            QTRY_COMPARE_WITH_TIMEOUT(fixture.window().visibility(), QWindow::Minimized, 5000);
        }

        fixture.videoItem().beginRenderShutdown();
        QTRY_VERIFY_WITH_TIMEOUT(coordinator->snapshot().renderReleased, 5000);
        QVERIFY2(
            coordinator->waitForRenderRelease(0ms),
            qPrintable(QStringLiteral("cycle %1 did not prove render release").arg(cycle)));

        const auto released = coordinator->snapshot();
        QCOMPARE(released.activeRenderSections, std::size_t{0});
        QCOMPARE(released.liveRenderContexts, std::size_t{0});

        core->close();
        QVERIFY(!core->isOpen());
        QCoreApplication::processEvents();
    }
}

} // namespace

int main(int argc, char* argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication application(argc, argv);
    MpvVideoShutdownTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "mpv_video_shutdown_test.moc"
