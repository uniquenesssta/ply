#include "playback_command_bus.h"

#include <QMetaObject>
#include <QString>

#include <mutex>
#include <utility>

namespace player::playback::application {

PlaybackCommandBus::PlaybackCommandBus(PlaybackSession& session, QObject* parent)
    : QObject(parent)
    , session_(&session)
{
}

bool PlaybackCommandBus::submit(
    player::playback::domain::PlaybackCommand command,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (player::playback::domain::validatePlaybackCommand(command).has_value()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback command failed domain validation.");
        }
        return false;
    }

    std::scoped_lock lock(gateMutex_);
    if (!acceptingCommands_) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback command bus is closed.");
        }
        return false;
    }

    const QPointer<PlaybackSession> guardedSession = session_;
    if (guardedSession.isNull()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("PlaybackSession is no longer available.");
        }
        return false;
    }

    const bool queued = QMetaObject::invokeMethod(
        guardedSession.data(),
        [guardedSession, command = std::move(command)]() mutable {
            if (!guardedSession.isNull()) {
                guardedSession->processCommand(std::move(command));
            }
        },
        Qt::QueuedConnection);

    if (!queued && errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to queue playback command to PlaybackSession.");
    }
    return queued;
}

void PlaybackCommandBus::close()
{
    std::scoped_lock lock(gateMutex_);
    acceptingCommands_ = false;
}

bool PlaybackCommandBus::isAcceptingCommands() const
{
    std::scoped_lock lock(gateMutex_);
    return acceptingCommands_ && !session_.isNull();
}

} // namespace player::playback::application
