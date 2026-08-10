#pragma once

#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickOpenGLUtils>
#include <QQuickWindow>
#include <QScreen>
#include <QWindow>

#include <mutex>

namespace player::test::render {
namespace detail {

inline void configureNonInteractiveWindow(
    QQuickWindow& window,
    QSize logicalSize,
    qreal opacity)
{
    window.setColor(Qt::black);
    window.setFlags(
        Qt::Tool
        | Qt::FramelessWindowHint
        | Qt::WindowDoesNotAcceptFocus
        | Qt::WindowTransparentForInput);
    window.setOpacity(opacity);
    window.setPersistentGraphics(false);
    window.setPersistentSceneGraph(false);
    window.resize(logicalSize);
}

inline void setItemSize(QQuickItem& item, QSize logicalSize)
{
    item.setWidth(logicalSize.width());
    item.setHeight(logicalSize.height());
}

inline void bindItemToWindow(QQuickWindow& window, QQuickItem& item)
{
    setItemSize(item, window.size());

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

inline void moveOffscreen(QQuickWindow& window)
{
    QScreen* screen = window.screen();
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen == nullptr) {
        return;
    }

    const QRect virtualGeometry = screen->virtualGeometry();
    window.setPosition(virtualGeometry.bottomRight() + QPoint(512, 512));
}

inline void releaseQuickWindow(QQuickWindow& window)
{
    window.hide();
    window.releaseResources();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

} // namespace detail

class QuickVideoSurfaceFixture final
{
public:
    explicit QuickVideoSurfaceFixture(QSize logicalSize)
        : videoItem_(window_.contentItem())
    {
        // Pixel-validation windows must remain fully opaque so grabWindow() observes
        // the actual composed video. Keep them non-interactive and outside the
        // virtual desktop instead of changing their rendered opacity.
        detail::configureNonInteractiveWindow(window_, logicalSize, 1.0);
        detail::bindItemToWindow(window_, videoItem_);
        detail::moveOffscreen(window_);
    }

    ~QuickVideoSurfaceFixture()
    {
        release();
    }

    QuickVideoSurfaceFixture(const QuickVideoSurfaceFixture&) = delete;
    QuickVideoSurfaceFixture& operator=(const QuickVideoSurfaceFixture&) = delete;

    [[nodiscard]] QQuickWindow& window() noexcept
    {
        return window_;
    }

    [[nodiscard]] player::playback::infrastructure::mpv::render::MpvVideoItem& videoItem() noexcept
    {
        return videoItem_;
    }

    void show()
    {
        detail::moveOffscreen(window_);
        window_.show();
        detail::moveOffscreen(window_);
        window_.update();
    }

    void resize(QSize logicalSize)
    {
        window_.resize(logicalSize);
        // QWindow native resize notification timing is platform-dependent. The
        // fixture owns the synthetic binding, so update the test item explicitly
        // while retaining width/height signal bindings for fullscreen/screen changes.
        detail::setItemSize(videoItem_, logicalSize);
        videoItem_.update();
        window_.update();
    }

    void moveOffscreen()
    {
        detail::moveOffscreen(window_);
    }

    void release()
    {
        if (released_) {
            return;
        }

        detail::releaseQuickWindow(window_);
        released_ = true;
    }

private:
    bool released_ = false;
    QQuickWindow window_;
    player::playback::infrastructure::mpv::render::MpvVideoItem videoItem_;
};

struct FramebufferGeometrySnapshot final
{
    QSize logicalSize;
    qreal devicePixelRatio = 1.0;
    QSize framebufferSize;
    int framebufferGeneration = 0;
};

class FramebufferGeometryProbe final
{
public:
    void recordPresentationState(
        const player::playback::infrastructure::mpv::render::MpvVideoPresentationState& state)
    {
        std::scoped_lock lock(mutex_);
        snapshot_.logicalSize = QSize(
            qRound(state.logicalSize.width()),
            qRound(state.logicalSize.height()));
        snapshot_.devicePixelRatio = state.devicePixelRatio;
    }

    void recordFramebufferSize(QSize size)
    {
        std::scoped_lock lock(mutex_);
        snapshot_.framebufferSize = size;
        ++snapshot_.framebufferGeneration;
    }

    [[nodiscard]] FramebufferGeometrySnapshot snapshot() const
    {
        std::scoped_lock lock(mutex_);
        return snapshot_;
    }

private:
    mutable std::mutex mutex_;
    FramebufferGeometrySnapshot snapshot_;
};

class GeometryProbeRenderer final : public QQuickFramebufferObject::Renderer
{
public:
    explicit GeometryProbeRenderer(FramebufferGeometryProbe& probe) noexcept
        : probe_(probe)
    {
    }

    void synchronize(QQuickFramebufferObject* item) override
    {
        const auto* videoItem = qobject_cast<
            player::playback::infrastructure::mpv::render::MpvVideoItem*>(item);
        if (videoItem != nullptr) {
            probe_.recordPresentationState(videoItem->presentationState());
        }
    }

    [[nodiscard]] QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override
    {
        probe_.recordFramebufferSize(size);
        return new QOpenGLFramebufferObject(size);
    }

    void render() override
    {
        QOpenGLContext* context = QOpenGLContext::currentContext();
        if (context != nullptr) {
            QOpenGLFunctions* functions = context->functions();
            if (functions != nullptr) {
                functions->glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
                functions->glClear(GL_COLOR_BUFFER_BIT);
            }
        }

        QQuickOpenGLUtils::resetOpenGLState();
    }

private:
    FramebufferGeometryProbe& probe_;
};

class GeometryProbeItem final
    : public player::playback::infrastructure::mpv::render::MpvVideoItem
{
public:
    GeometryProbeItem(FramebufferGeometryProbe& probe, QQuickItem* parent)
        : MpvVideoItem(parent)
        , probe_(probe)
    {
    }

    [[nodiscard]] Renderer* createRenderer() const override
    {
        return new GeometryProbeRenderer(probe_);
    }

private:
    FramebufferGeometryProbe& probe_;
};

class QuickFramebufferGeometryFixture final
{
public:
    explicit QuickFramebufferGeometryFixture(QSize logicalSize)
        : videoItem_(probe_, window_.contentItem())
    {
        // Geometry-only tests do not inspect composed pixels, so near-transparent
        // native windows keep fullscreen/DPI coverage unobtrusive.
        detail::configureNonInteractiveWindow(window_, logicalSize, 0.001);
        detail::bindItemToWindow(window_, videoItem_);
    }

    ~QuickFramebufferGeometryFixture()
    {
        release();
    }

    QuickFramebufferGeometryFixture(const QuickFramebufferGeometryFixture&) = delete;
    QuickFramebufferGeometryFixture& operator=(const QuickFramebufferGeometryFixture&) = delete;

    [[nodiscard]] QQuickWindow& window() noexcept
    {
        return window_;
    }

    [[nodiscard]] GeometryProbeItem& videoItem() noexcept
    {
        return videoItem_;
    }

    void show()
    {
        window_.show();
        window_.update();
    }

    void resize(QSize logicalSize)
    {
        window_.resize(logicalSize);
        detail::setItemSize(videoItem_, logicalSize);
        videoItem_.update();
        window_.update();
    }

    void moveOffscreen()
    {
        detail::moveOffscreen(window_);
    }

    [[nodiscard]] QSize expectedPhysicalFramebufferSize() const
    {
        return QSize(
            qRound(videoItem_.width() * window_.effectiveDevicePixelRatio()),
            qRound(videoItem_.height() * window_.effectiveDevicePixelRatio()));
    }

    [[nodiscard]] FramebufferGeometrySnapshot geometrySnapshot() const
    {
        return probe_.snapshot();
    }

    [[nodiscard]] bool geometryMatches() const
    {
        const FramebufferGeometrySnapshot snapshot = probe_.snapshot();
        const QSize logicalSize(qRound(videoItem_.width()), qRound(videoItem_.height()));
        return snapshot.logicalSize == logicalSize
            && snapshot.framebufferSize == expectedPhysicalFramebufferSize()
            && qFuzzyCompare(snapshot.devicePixelRatio, window_.effectiveDevicePixelRatio());
    }

    void release()
    {
        if (released_) {
            return;
        }

        detail::releaseQuickWindow(window_);
        released_ = true;
    }

private:
    bool released_ = false;
    QQuickWindow window_;
    FramebufferGeometryProbe probe_;
    GeometryProbeItem videoItem_;
};

} // namespace player::test::render
