#include "playback_session_thread.h"

#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/session/playback_session.h"

#include <QMetaObject>

namespace player::playback::application {

PlaybackSessionThread::PlaybackSessionThread(QObject* parent)
    : QObject(parent)
{
    thread_.setObjectName(QStringLiteral("PlaybackThread"));
}

PlaybackSessionThread::~PlaybackSessionThread()
{
    QString ignored;
    if (!stop(&ignored) && thread_.isRunning()) {
        thread_.quit();
        thread_.wait();
    }
}

bool PlaybackSessionThread::start(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (thread_.isRunning()) {
        return true;
    }

    if (!session_.isNull()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Previous PlaybackSession has not been released yet.");
        }
        return false;
    }

    commandBus_.reset();

    auto* session = new PlaybackSession();
    session->moveToThread(&thread_);
    session_ = session;
    commandBus_ = std::make_unique<PlaybackCommandBus>(*session);

    QObject::connect(
        &thread_,
        &QThread::started,
        session,
        &PlaybackSession::initialize,
        Qt::QueuedConnection);
    QObject::connect(
        &thread_,
        &QThread::finished,
        session,
        &QObject::deleteLater);
    QObject::connect(
        session,
        &PlaybackSession::ready,
        this,
        &PlaybackSessionThread::ready,
        Qt::QueuedConnection);
    QObject::connect(
        session,
        &PlaybackSession::startupFailed,
        this,
        &PlaybackSessionThread::startupFailed,
        Qt::QueuedConnection);
    QObject::connect(
        session,
        &PlaybackSession::stopped,
        this,
        &PlaybackSessionThread::stopped,
        Qt::QueuedConnection);
    QObject::connect(
        session,
        &PlaybackSession::startupFailed,
        session,
        [this](const QString&) {
            thread_.quit();
        },
        Qt::DirectConnection);
    QObject::connect(
        session,
        &PlaybackSession::stopped,
        session,
        [this]() {
            thread_.quit();
        },
        Qt::DirectConnection);

    thread_.start();
    return true;
}

bool PlaybackSessionThread::stop(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (QThread::currentThread() == &thread_) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("PlaybackSessionThread::stop cannot wait on the playback thread itself.");
        }
        return false;
    }

    if (!thread_.isRunning()) {
        commandBus_.reset();
        return true;
    }

    const QPointer<PlaybackSession> guardedSession = session_;
    if (!guardedSession.isNull()) {
        const bool queued = QMetaObject::invokeMethod(
            guardedSession.data(),
            [guardedSession]() {
                if (!guardedSession.isNull()) {
                    guardedSession->shutdown();
                }
            },
            Qt::QueuedConnection);
        if (!queued) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Failed to queue PlaybackSession shutdown.");
            }
            return false;
        }
    } else {
        thread_.quit();
    }

    if (!thread_.wait(5000)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback thread did not stop within 5 seconds.");
        }
        return false;
    }

    commandBus_.reset();
    return true;
}

bool PlaybackSessionThread::isRunning() const noexcept
{
    return thread_.isRunning();
}

PlaybackCommandBus* PlaybackSessionThread::commandBus() noexcept
{
    return commandBus_.get();
}

} // namespace player::playback::application
