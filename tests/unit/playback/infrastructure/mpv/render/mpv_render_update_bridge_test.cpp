#include "playback/infrastructure/mpv/render/mpv_render_context.h"
#include "playback/infrastructure/mpv/render/mpv_render_update_bridge.h"
#include "render_test_fixture.h"

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QThread>
#include <QtTest>

#include <cstdint>
#include <memory>
#include <thread>

namespace {

using player::playback::infrastructure::mpv::render::MpvRenderContext;
using player::playback::infrastructure::mpv::render::MpvRenderUpdateBridge;
using player::playback::mpv::MpvHandle;
using player::test::render::OffscreenOpenGlContext;
using player::test::render::createInitializedCore;
using player::test::render::createInitializedVideoCore;

bool writeGeneratedY4mVideo(const QString& path, int frameCount, QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    const QByteArray header = QByteArrayLiteral("YUV4MPEG2 W16 H16 F30:1 Ip A1:1 C420jpeg\n");
    if (file.write(header) != header.size()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to write the generated Y4M header.");
        }
        return false;
    }

    QByteArray yPlane(16 * 16, '\0');
    const QByteArray uPlane(8 * 8, static_cast<char>(128));
    const QByteArray vPlane(8 * 8, static_cast<char>(128));
    const QByteArray frameHeader = QByteArrayLiteral("FRAME\n");

    for (int frame = 0; frame < frameCount; ++frame) {
        yPlane.fill(static_cast<char>(16 + ((frame * 7) % 200)));

        if (file.write(frameHeader) != frameHeader.size()
            || file.write(yPlane) != yPlane.size()
            || file.write(uPlane) != uPlane.size()
            || file.write(vPlane) != vPlane.size()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Unable to write generated Y4M frame %1.").arg(frame);
            }
            return false;
        }
    }

    return true;
}

int runLoadFileOnCoreThread(mpv_handle* handle, const QString& path)
{
    int result = MPV_ERROR_GENERIC;
    const QByteArray source = path.toUtf8();

    std::thread commandThread([handle, source, &result] {
        const char* command[] = {
            "loadfile",
            source.constData(),
            nullptr,
        };
        result = mpv_command(handle, command);
    });
    commandThread.join();

    return result;
}

int runStopOnCoreThread(mpv_handle* handle)
{
    int result = MPV_ERROR_GENERIC;

    std::thread commandThread([handle, &result] {
        const char* command[] = {
            "stop",
            nullptr,
        };
        result = mpv_command(handle, command);
    });
    commandThread.join();

    return result;
}

class MpvRenderUpdateBridgeTest final : public QObject {
    Q_OBJECT

private slots:
    void callbackIsQueuedOntoBridgeThread();
    void deactivateSuppressesQueuedRequests();
    void repeatedActivationIsStable();
    void generatedVideoProducesRepeatedUpdateRequests();
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
        writeGeneratedY4mVideo(videoPath, 60, &mediaError),
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
    QCOMPARE(runLoadFileOnCoreThread(core->nativeHandle(), videoPath), 0);

    QTRY_VERIFY_WITH_TIMEOUT(frameUpdateCount >= 3 || !updateError.isEmpty(), 5000);
    QVERIFY2(updateError.isEmpty(), updateError.toLocal8Bit().constData());
    QVERIFY(updateRequestCount >= 3);
    QVERIFY(frameUpdateCount >= 3);

    QCOMPARE(runStopOnCoreThread(core->nativeHandle()), 0);
    QVERIFY2(bridge.deactivate(&errorMessage), errorMessage.toLocal8Bit().constData());
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
