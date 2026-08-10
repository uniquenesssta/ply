#pragma once

#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QImage>
#include <QQuickWindow>

namespace player::test::render {

class QuickVideoPixelCaptureFixture final
{
public:
    explicit QuickVideoPixelCaptureFixture(QSize logicalSize)
        : videoItem_(window_.contentItem())
    {
        window_.setColor(Qt::black);
        resize(logicalSize);
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

    void create()
    {
        if (created_) {
            return;
        }

        window_.create();
        created_ = true;
    }

    void resize(QSize logicalSize)
    {
        window_.resize(logicalSize);
        videoItem_.setWidth(logicalSize.width());
        videoItem_.setHeight(logicalSize.height());
        videoItem_.update();
    }

    [[nodiscard]] QImage grabWindow()
    {
        create();
        return window_.grabWindow();
    }

    void release()
    {
        if (released_) {
            return;
        }

        window_.releaseResources();
        window_.destroy();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        released_ = true;
    }

private:
    bool created_ = false;
    bool released_ = false;
    QQuickWindow window_;
    player::playback::infrastructure::mpv::render::MpvVideoItem videoItem_;
};

} // namespace player::test::render
