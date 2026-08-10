#pragma once

#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QCoreApplication>
#include <QImage>
#include <QQuickWindow>

namespace player::test::render {

class QuickVideoPixelCaptureFixture final
{
public:
    explicit QuickVideoPixelCaptureFixture(QSize logicalSize)
        : videoItem_(window_.contentItem())
    {
        // Keep this fixture deliberately equivalent to the R4-05 pixel smoke that
        // passed on Windows: ordinary visible QQuickWindow, no special flags,
        // opacity changes, off-screen moves, or pre-show update scheduling.
        window_.setColor(Qt::black);
        window_.resize(logicalSize);
        videoItem_.setWidth(logicalSize.width());
        videoItem_.setHeight(logicalSize.height());
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

        window_.show();
        window_.update();
        shown_ = true;
    }

    void resize(QSize logicalSize)
    {
        window_.resize(logicalSize);
        videoItem_.setWidth(logicalSize.width());
        videoItem_.setHeight(logicalSize.height());
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
        QCoreApplication::processEvents();
        released_ = true;
    }

private:
    bool shown_ = false;
    bool released_ = false;
    QQuickWindow window_;
    player::playback::infrastructure::mpv::render::MpvVideoItem videoItem_;
};

} // namespace player::test::render
