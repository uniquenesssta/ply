#include "playback/infrastructure/mpv/render/mpv_video_item.h"
#include "render_test_fixture.h"
#include "render_video_fixture.h"

#include <QColor>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSGTexture>
#include <QSGTextureProvider>
#include <QTemporaryDir>
#include <QWindow>
#include <QtTest>

#include <memory>
#include <mutex>

namespace {

using player::playback::infrastructure::mpv::render::MpvVideoItem;
using player::playback::mpv::MpvHandle;
using player::test::render::GeneratedY4mPattern;
using player::test::render::createInitializedVideoCore;
using player::test::render::loadFileOnWorkerThread;
using player::test::render::stopPlaybackOnWorkerThread;
using player::test::render::writeGeneratedY4mVideo;

class FramebufferSizeProbe final
{
public:
    void sample(MpvVideoItem& item)
    {
        QSGTextureProvider* provider = item.textureProvider();
        if (provider == nullptr) {
            return;
        }

        QSGTexture* texture = provider->texture();
        if (texture == nullptr) {
            return;
        }

        std::scoped_lock lock(mutex_);
        size_ = texture->textureSize();
    }

    [[nodiscard]] QSize size() const
    {
        std::scoped_lock lock(mutex_);
        return size_;
    }

private:
    mutable std::mutex mutex_;
    QSize size_;
};

void bindItemToWindow(QQuickWindow& window, MpvVideoItem& item)
{
    item.setWidth(window.width());
    item.setHeight(window.height());

    QObject::connect(
        &window,
        &QWindow::widthChanged,
        &item,
        [&item](int width) {
            item.setWidth(width);
        });
    QObject::connect(
        &window,
        &QWindow::heightChanged,
        &item,
        [&item](int height) {
            item.setHeight(height);
        });
}

void attachFramebufferProbe(
    QQuickWindow& window,
    MpvVideoItem& item,
    FramebufferSizeProbe& probe)
{
    QObject::connect(
        &window,
        &QQuickWindow::afterRendering,
        &item,
        [&item, &probe] {
            probe.sample(item);
        },
        Qt::DirectConnection);
}

QSize expectedPhysicalFramebufferSize(const MpvVideoItem& item)
{
    const auto state = item.presentationState();
    return QSize(
        qRound(state.logicalSize.width() * state.devicePixelRatio),
        qRound(state.logicalSize.height() * state.devicePixelRatio));
}

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

void releaseWindowResources(QQuickWindow& window)
{
    window.hide();
    window.releaseResources();
    QCoreApplication::processEvents();
}

class MpvVideoResizeDpiTest final : public QObject
{
    Q_OBJECT

private slots:
    void framebufferTracksLogicalResizeAtEffectiveDpr();
    void squareVideoKeepsAspectAcrossResize();
    void fullscreenUsesPhysicalFramebufferResolution();
};

void MpvVideoResizeDpiTest::framebufferTracksLogicalResizeAtEffectiveDpr()
{
    FramebufferSizeProbe probe;
    QQuickWindow window;
    window.setColor(Qt::black);
    window.resize(160, 96);

    MpvVideoItem videoItem(window.contentItem());
    bindItemToWindow(window, videoItem);
    attachFramebufferProbe(window, videoItem, probe);

    window.show();
    window.update();

    QTRY_VERIFY_WITH_TIMEOUT(window.isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        probe.size() == expectedPhysicalFramebufferSize(videoItem),
        5000);

    auto state = videoItem.presentationState();
    QCOMPARE(state.logicalSize, QSizeF(160.0, 96.0));
    QCOMPARE(state.devicePixelRatio, window.effectiveDevicePixelRatio());
    QVERIFY(state.devicePixelRatio > 0.0);

    const QSize initialFramebufferSize = probe.size();

    window.resize(320, 192);
    window.update();

    QTRY_COMPARE_WITH_TIMEOUT(window.size(), QSize(320, 192), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        videoItem.presentationState().logicalSize == QSizeF(320.0, 192.0),
        3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        probe.size() == expectedPhysicalFramebufferSize(videoItem),
        5000);

    state = videoItem.presentationState();
    QCOMPARE(state.devicePixelRatio, window.effectiveDevicePixelRatio());
    QVERIFY(probe.size() != initialFramebufferSize);

    releaseWindowResources(window);
}

void MpvVideoResizeDpiTest::squareVideoKeepsAspectAcrossResize()
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

    FramebufferSizeProbe probe;
    QQuickWindow window;
    window.setColor(Qt::black);
    window.resize(160, 90);

    MpvVideoItem videoItem(window.contentItem());
    bindItemToWindow(window, videoItem);
    attachFramebufferProbe(window, videoItem, probe);
    videoItem.setRenderCoreHandle(core->nativeHandle());

    window.show();
    window.update();

    QTRY_VERIFY_WITH_TIMEOUT(window.isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        probe.size() == expectedPhysicalFramebufferSize(videoItem),
        5000);

    QCOMPARE(loadFileOnWorkerThread(core->nativeHandle(), videoPath), 0);
    QTRY_VERIFY_WITH_TIMEOUT(hasUndistortedSquareVideo(window.grabWindow()), 6000);

    window.resize(320, 180);
    window.update();

    QTRY_COMPARE_WITH_TIMEOUT(window.size(), QSize(320, 180), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        probe.size() == expectedPhysicalFramebufferSize(videoItem),
        5000);
    QTRY_VERIFY_WITH_TIMEOUT(hasUndistortedSquareVideo(window.grabWindow()), 6000);

    QCOMPARE(stopPlaybackOnWorkerThread(core->nativeHandle()), 0);
    videoItem.setRenderCoreHandle(nullptr);
    videoItem.update();

    releaseWindowResources(window);
}

void MpvVideoResizeDpiTest::fullscreenUsesPhysicalFramebufferResolution()
{
    if (qEnvironmentVariable("PLAYER_R4_06_FULLSCREEN_SMOKE") != QStringLiteral("1")) {
        QSKIP("Fullscreen smoke runs once in the 100% scale test process.");
    }

    FramebufferSizeProbe probe;
    QQuickWindow window;
    window.setColor(Qt::black);
    window.resize(320, 180);

    MpvVideoItem videoItem(window.contentItem());
    bindItemToWindow(window, videoItem);
    attachFramebufferProbe(window, videoItem, probe);

    window.show();
    window.update();

    QTRY_VERIFY_WITH_TIMEOUT(window.isExposed(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        probe.size() == expectedPhysicalFramebufferSize(videoItem),
        5000);

    window.showFullScreen();
    window.update();

    QTRY_VERIFY_WITH_TIMEOUT(window.visibility() == QWindow::FullScreen, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        videoItem.presentationState().logicalSize
            == QSizeF(window.width(), window.height()),
        5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        probe.size() == expectedPhysicalFramebufferSize(videoItem),
        8000);

    window.showNormal();
    window.update();

    QTRY_VERIFY_WITH_TIMEOUT(window.visibility() != QWindow::FullScreen, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        probe.size() == expectedPhysicalFramebufferSize(videoItem),
        5000);

    releaseWindowResources(window);
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
