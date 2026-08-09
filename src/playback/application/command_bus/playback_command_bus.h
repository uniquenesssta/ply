#pragma once

#include "playback/application/session/playback_session.h"
#include "playback/domain/commands/playback_command.h"

#include <QObject>
#include <QPointer>

#include <mutex>

class QString;

namespace player::playback::application {

class PlaybackCommandBus final : public QObject
{
public:
    explicit PlaybackCommandBus(PlaybackSession& session, QObject* parent = nullptr);

    PlaybackCommandBus(const PlaybackCommandBus&) = delete;
    PlaybackCommandBus& operator=(const PlaybackCommandBus&) = delete;

    [[nodiscard]] bool submit(
        player::playback::domain::PlaybackCommand command,
        QString* errorMessage = nullptr);

    void close();
    [[nodiscard]] bool isAcceptingCommands() const;

private:
    mutable std::mutex gateMutex_;
    QPointer<PlaybackSession> session_;
    bool acceptingCommands_ = true;
};

} // namespace player::playback::application
