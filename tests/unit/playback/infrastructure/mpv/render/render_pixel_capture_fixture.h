#pragma once

#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QScreen>

namespace player::test::render {
namespace detail {

inline void movePixelWindowToScreenCorner(QQuickWindow& window)
{
    QScreen* screen = window.screen();
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen == nullptr) {
        return;
    }

    const QRect availableGeometry = screen->availableGeometry();
    if (!availableGeometry.isValid()) {
        return;
    }

    constexpr int margin = 8;
    const int x = qMax(
        availableGeometry.left(),
        availableGeometry.right() - window.width() + 1 - margin);
    const int y = qMax(
        availableGeometry.top(),
        availableGeometry.bottom() - window.height() + 1 - margin);
    window.setPosition(x, y);
}

} // namespace detail

class QuickVideoPixelCaptureFixture final
{
public:
    explicit QuickVideoPixelCaptureFixture(QSize logicalSize)
        : videoItem_(window_.contentItem())
    {
        window_.setColor(Qt::black);
        resize(logicalSize);
        detail::movePixelWindowToScreenCorner(window_);
    }

    ~QuickVideoPixelCaptureFixture()
    {
        release();
    }

    QuickVideoPixelCaptureFixture(const QuickVideoPixelCaptureFixture&) = delete;
    QuickVideoPixelCaptureFixture& operator=(const QuickVideoPixelCaptureFixture&) = delete;

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
        if (shown_) {
            return;
        }

        detail::movePixelWindowToScreenCorner(window_);
        window_.show();
        detail::movePixelWindowToScreenCorner(window_);
        window_.update();
        shown_ = true;
    }

    void resize(QSize logicalSize)
    {
        window_.resize(logicalSize);
        videoItem_.setWidth(logicalSize.width());
        videoItem_.setHeight(logicalSize.height());
        detail::movePixelWindowToScreenCorner(window_);
        videoItem_.update();
        window_.update();
    }

    [[nodiscard]] QImage grabWindow()
    {
        return window_.grabWindow();
    }

    void release()
    {
        if (released_) {
            return;
        }

        window_.hide();
        window_.releaseResources();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        released_ = true;
    }

private:
    bool shown_ = false;
    bool released_ = false;
    QQuickWindow window_;
    player::playback::infrastructure::mpv::render::MpvVideoItem videoItem_;
};

} // namespace player::test::render
