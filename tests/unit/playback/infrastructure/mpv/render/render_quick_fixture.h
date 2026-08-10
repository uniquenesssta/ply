#pragma once

#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QCoreApplication>
#include <QQuickWindow>
#include <QSGTexture>
#include <QSGTextureProvider>
#include <QWindow>

#include <mutex>

namespace player::test::render {

class QuickVideoSurfaceFixture final
{
public:
    explicit QuickVideoSurfaceFixture(QSize logicalSize)
        : videoItem_(window_.contentItem())
    {
        window_.setColor(Qt::black);
        window_.resize(logicalSize);

        videoItem_.setWidth(window_.width());
        videoItem_.setHeight(window_.height());

        QObject::connect(
            &window_,
            &QWindow::widthChanged,
            &videoItem_,
            [this](int width) {
                videoItem_.setWidth(width);
            });
        QObject::connect(
            &window_,
            &QWindow::heightChanged,
            &videoItem_,
            [this](int height) {
                videoItem_.setHeight(height);
            });
        QObject::connect(
            &window_,
            &QQuickWindow::afterRendering,
            &videoItem_,
            [this] {
                sampleFramebufferSize();
            },
            Qt::DirectConnection);
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
        window_.show();
        window_.update();
    }

    void resize(QSize logicalSize)
    {
        window_.resize(logicalSize);
        window_.update();
    }

    [[nodiscard]] QSize framebufferSize() const
    {
        std::scoped_lock lock(framebufferMutex_);
        return framebufferSize_;
    }

    [[nodiscard]] QSize expectedPhysicalFramebufferSize() const
    {
        const auto state = videoItem_.presentationState();
        return QSize(
            qRound(state.logicalSize.width() * state.devicePixelRatio),
            qRound(state.logicalSize.height() * state.devicePixelRatio));
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
    void sampleFramebufferSize()
    {
        QSGTextureProvider* provider = videoItem_.textureProvider();
        if (provider == nullptr) {
            return;
        }

        QSGTexture* texture = provider->texture();
        if (texture == nullptr) {
            return;
        }

        std::scoped_lock lock(framebufferMutex_);
        framebufferSize_ = texture->textureSize();
    }

    mutable std::mutex framebufferMutex_;
    QSize framebufferSize_;
    bool released_ = false;
    QQuickWindow window_;
    player::playback::infrastructure::mpv::render::MpvVideoItem videoItem_;
};

} // namespace player::test::render
