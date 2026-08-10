#include "playback/infrastructure/mpv/render/mpv_render_context.h"
#include "playback/infrastructure/mpv/render/mpv_render_shutdown_coordinator.h"
#include "playback/infrastructure/mpv/render/mpv_render_update_bridge.h"
#include "render_test_fixture.h"
#include "render_video_fixture.h"

#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QThread>
#include <QtTest>

#include <cstdint>
#include <memory>

namespace {

using player::playback::infrastructure::mpv::render::MpvRenderContext;
using player::playback::infrastructure::mpv::render::MpvRenderShutdownCoordinator;
using player::playback::infrastructure::mpv::render::MpvRenderUpdateBridge;
using player::playback::mpv::MpvHandle;
using player::test::render::GeneratedY4mPattern;
using player::test::render::OffscreenOpenGlContext;
using player::test::render::createInitializedCore;
using player::test::render::createInitializedVideoCore;
using player::test::render::loadFileOnWorkerThread;
using player::test::render::stopPlaybackOnWorkerThread;
using player::test::render::writeGeneratedY4mVideo;

class MpvRenderUpdateBridgeTest final : public QObject {
    Q_OBJECT

private slots:
    void callbackIsQueuedOntoBridgeThread();
    void deactivateSuppressesQueuedRequests();
    void repeatedActivationIsStable();
    void generatedVideoProducesRepeatedUpdateRequests();
    void shutdownCoordinatorSuppressesLateRequests();
};

void MpvRenderUpdateBridgeTest::callbackIsQueuedOntoBridgeThread()
{
    OffscreenOpenGlContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QString errorMessage;
    std::unique_ptr<MpvRenderContext> renderContext =
        MpvRenderContext::create(core->nativeHandle(), &errorMessage);
    QVERIFY2(renderContext != nullptr, errorMessage.toLocal8Bit().constData());

    MpvRenderUpdateBridge bridge;
    QSignalSpy updateSpy(&bridge, &MpvRenderUpdateBridge::updateRequested);
    QThread* deliveryThread = nullptr;
    connect(
        &bridge,
        &MpvRenderUpdateBridge::updateRequested,
        &bridge,
        [&deliveryThread] {
            deliveryThread = QThread::currentThread();
        });

    fixture.doneCurrent();
    QVERIFY2(bridge.activate(*renderContext, &errorMessage), errorMessage.toLocal8Bit().constData());
    QVERIFY(bridge.isActive());

    QTRY_VERIFY_WITH_TIMEOUT(updateSpy.count() >= 1, 2000);
    QCOMPARE(deliveryThread, QThread::currentThread());

    QVERIFY2(bridge.deactivate(&errorMessage), errorMessage.toLocal8Bit().constData());
    QVERIFY(!bridge.isActive());

    QVERIFY2(fixture.makeCurrent(&fixtureError), fixtureError.toLocal8Bit().constData());
    QVERIFY2(renderContext->close(&errorMessage), errorMessage.toLocal8Bit().constData());
}

void MpvRenderUpdateBridgeTest::deactivateSuppressesQueuedRequests()
{
    OffscreenOpenGlContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QString errorMessage;
    std::unique_ptr<MpvRenderContext> renderContext =
        MpvRenderContext::create(core->nativeHandle(), &errorMessage);
    QVERIFY2(renderContext != nullptr, errorMessage.toLocal8Bit().constData());

    MpvRenderUpdateBridge bridge;
    QSignalSpy updateSpy(&bridge, &MpvRenderUpdateBridge::updateRequested);

    QVERIFY2(bridge.activate(*renderContext, &errorMessage), errorMessage.toLocal8Bit().constData());
    QVERIFY2(bridge.deactivate(&errorMessage), errorMessage.toLocal8Bit().constData());

    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QTest::qWait(25);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QCOMPARE(updateSpy.count(), 0);

    QVERIFY2(renderContext->close(&errorMessage), errorMessage.toLocal8Bit().constData());
}

void MpvRenderUpdateBridgeTest::repeatedActivationIsStable()
{
    OffscreenOpenGlContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QString errorMessage;
    std::unique_ptr<MpvRenderContext> renderContext =
        MpvRenderContext::create(core->nativeHandle(), &errorMessage);
    QVERIFY2(renderContext != nullptr, errorMessage.toLocal8Bit().constData());

    MpvRenderUpdateBridge bridge;
    QSignalSpy updateSpy(&bridge, &MpvRenderUpdateBridge::updateRequested);

    for (int iteration = 0; iteration < 10; ++iteration) {
        const int countBeforeActivation = updateSpy.count();

        QVERIFY2(bridge.activate(*renderContext, &errorMessage), errorMessage.toLocal8Bit().constData());
        QTRY_VERIFY_WITH_TIMEOUT(updateSpy.count() > countBeforeActivation, 2000);
        QVERIFY2(bridge.deactivate(&errorMessage), errorMessage.toLocal8Bit().constData());
        QVERIFY(!bridge.isActive());
    }

    QVERIFY2(renderContext->close(&errorMessage), errorMessage.toLocal8Bit().constData());
}

void MpvRenderUpdateBridgeTest::generatedVideoProducesRepeatedUpdateRequests()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("render-update-bridge.y4m"));
    QString mediaError;
    QVERIFY2(
        writeGeneratedY4mVideo(
            videoPath,
            60,
            GeneratedY4mPattern::AnimatedGray,
            &mediaError),
        mediaError.toLocal8Bit().constData());

    OffscreenOpenGlContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QString errorMessage;
    std::unique_ptr<MpvRenderContext> renderContext =
        MpvRenderContext::create(core->nativeHandle(), &errorMessage);
    QVERIFY2(renderContext != nullptr, errorMessage.toLocal8Bit().constData());

    MpvRenderUpdateBridge bridge;
    int updateRequestCount = 0;
    int frameUpdateCount = 0;
    QString updateError;

    mpv_opengl_fbo target{0, 16, 16, 0};
    int flipY = 0;
    mpv_render_param renderParams[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &target},
        {MPV_RENDER_PARAM_FLIP_Y, &flipY},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    connect(
        &bridge,
        &MpvRenderUpdateBridge::updateRequested,
        &bridge,
        [&] {
            ++updateRequestCount;

            std::uint64_t updateFlags = 0;
            QString localError;
            if (!renderContext->update(&updateFlags, &localError)) {
                updateError = localError;
                return;
            }

            if ((updateFlags & MPV_RENDER_UPDATE_FRAME) != 0U) {
                if (!renderContext->render(renderParams, &localError)) {
                    updateError = localError;
                    return;
                }
                ++frameUpdateCount;
            }
        });

    QVERIFY2(bridge.activate(*renderContext, &errorMessage), errorMessage.toLocal8Bit().constData());
    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);

    QTRY_VERIFY_WITH_TIMEOUT(frameUpdateCount >= 3 || !updateError.isEmpty(), 5000);
    QVERIFY2(updateError.isEmpty(), updateError.toLocal8Bit().constData());
    QVERIFY(updateRequestCount >= 3);
    QVERIFY(frameUpdateCount >= 3);

    QCOMPARE(stopPlaybackOnWorkerThread(core->nativeHandle()), 0);
    QVERIFY2(bridge.deactivate(&errorMessage), errorMessage.toLocal8Bit().constData());
    QVERIFY2(renderContext->close(&errorMessage), errorMessage.toLocal8Bit().constData());
}

void MpvRenderUpdateBridgeTest::shutdownCoordinatorSuppressesLateRequests()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString videoPath = temporaryDir.filePath(QStringLiteral("render-update-shutdown.y4m"));
    QString mediaError;
    QVERIFY2(
        writeGeneratedY4mVideo(
            videoPath,
            180,
            GeneratedY4mPattern::AnimatedGray,
            &mediaError),
        mediaError.toLocal8Bit().constData());

    OffscreenOpenGlContext fixture;
    QString fixtureError;
    QVERIFY2(fixture.initialize(&fixtureError), fixtureError.toLocal8Bit().constData());

    QString coreError;
    std::unique_ptr<MpvHandle> core = createInitializedVideoCore(&coreError);
    QVERIFY2(core != nullptr, coreError.toLocal8Bit().constData());

    QString errorMessage;
    std::unique_ptr<MpvRenderContext> renderContext =
        MpvRenderContext::create(core->nativeHandle(), &errorMessage);
    QVERIFY2(renderContext != nullptr, errorMessage.toLocal8Bit().constData());

    auto shutdownCoordinator = std::make_shared<MpvRenderShutdownCoordinator>();
    MpvRenderUpdateBridge bridge({}, shutdownCoordinator, nullptr);
    QSignalSpy updateSpy(&bridge, &MpvRenderUpdateBridge::updateRequested);
    int frameUpdateCount = 0;
    QString updateError;

    mpv_opengl_fbo target{0, 16, 16, 0};
    int flipY = 0;
    mpv_render_param renderParams[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &target},
        {MPV_RENDER_PARAM_FLIP_Y, &flipY},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    connect(
        &bridge,
        &MpvRenderUpdateBridge::updateRequested,
        &bridge,
        [&] {
            std::uint64_t updateFlags = 0;
            QString localError;
            if (!renderContext->update(&updateFlags, &localError)) {
                updateError = localError;
                return;
            }

            if ((updateFlags & MPV_RENDER_UPDATE_FRAME) != 0U) {
                if (!renderContext->render(renderParams, &localError)) {
                    updateError = localError;
                    return;
                }
                ++frameUpdateCount;
            }
        });

    QVERIFY2(bridge.activate(*renderContext, &errorMessage), errorMessage.toLocal8Bit().constData());
    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);
    QTRY_VERIFY_WITH_TIMEOUT(frameUpdateCount >= 3 || !updateError.isEmpty(), 5000);
    QVERIFY2(updateError.isEmpty(), updateError.toLocal8Bit().constData());

    QVERIFY(shutdownCoordinator->beginShutdown());
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    const int countAfterShutdown = updateSpy.count();

    QCOMPARE(stopPlaybackOnWorkerThread(core->nativeHandle()), 0);
    QTest::qWait(150);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QCOMPARE(updateSpy.count(), countAfterShutdown);

    QVERIFY2(bridge.deactivate(&errorMessage), errorMessage.toLocal8Bit().constData());
    QVERIFY(!bridge.isActive());
    QVERIFY2(renderContext->close(&errorMessage), errorMessage.toLocal8Bit().constData());
}

} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    MpvRenderUpdateBridgeTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "mpv_render_update_bridge_test.moc"
