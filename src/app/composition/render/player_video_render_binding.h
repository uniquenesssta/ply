#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QtGlobal>

#include <memory>

namespace player::playback::application {
class PlaybackSessionThread;
}

namespace player::playback::infrastructure::mpv::render {
class MpvRenderShutdownCoordinator;
class MpvVideoItem;
}

namespace player::app {

class PlayerVideoRenderBinding final : public QObject
{
public:
    explicit PlayerVideoRenderBinding(
        player::playback::application::PlaybackSessionThread& playbackThread,
        QObject* parent = nullptr);
    ~PlayerVideoRenderBinding() override;

    [[nodiscard]] bool resetForStart(QString* errorMessage = nullptr);
    [[nodiscard]] bool attachRoot(QObject* qmlRoot, QString* errorMessage = nullptr);
    void beginShutdown() noexcept;
    [[nodiscard]] bool waitForRenderRelease(QString* errorMessage = nullptr);

private:
    void bindCoreIfReady() noexcept;

    QPointer<player::playback::infrastructure::mpv::render::MpvVideoItem> videoItem_;
    std::shared_ptr<const player::playback::infrastructure::mpv::render::MpvRenderShutdownCoordinator>
        shutdownCoordinator_;
    quintptr coreAddress_ = 0;
    bool shutdownStarted_ = false;
};

} // namespace player::app
