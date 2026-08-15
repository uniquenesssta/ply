#include "playback_session_thread.h"

#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/session/playback_session.h"
#include "playback/application/state_publisher/state_publisher.h"

#include <QMetaObject>
#include <QtGlobal>

namespace player::playback::application {
namespace {

constexpr unsigned long kShutdownWaitMilliseconds = 5000;

} // namespace

PlaybackSessionThread::PlaybackSessionThread(QObject* parent)
    : QObject(parent)
    , statePublisher_(new StatePublisher(this))
{
    thread_.setObjectName(QStringLiteral("PlaybackThread"));
}

PlaybackSessionThread::~PlaybackSessionThread()
{
    QString diagnostic;
    if (!stop(&diagnostic)) {
        qFatal("PlaybackSessionThread destruction failed to complete bounded playback shutdown.");
    }
}

bool PlaybackSessionThread::start(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (QThread::currentThread() != this->thread()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("PlaybackSessionThread::start must run on the host object's owning thread.");
        }
        return false;
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
    statePublisher_->reset();

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
        &PlaybackSession::snapshotCommitted,
        statePublisher_,
        &StatePublisher::acceptSnapshot,
        Qt::QueuedConnection);
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
        &PlaybackSession::requestFailed,
        this,
        &PlaybackSessionThread::requestFailed,
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
            if (commandBus_ != nullptr) {
                commandBus_->close();
            }
            thread_.quit();
        },
        Qt::DirectConnection);
    QObject::connect(
        session,
        &PlaybackSession::stopped,
        session,
        [this]() {
            if (commandBus_ != nullptr) {
                commandBus_->close();
            }
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

    if (QThread::currentThread() != this->thread()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("PlaybackSessionThread::stop must run on the host object's owning thread.");
        }
        return false;
    }

    if (commandBus_ != nullptr) {
        commandBus_->close();
    }

    if (!thread_.isRunning()) {
        commandBus_.reset();
        if (!session_.isNull()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Playback thread is stopped but PlaybackSession is still alive.");
            }
            return false;
        }
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

    if (!thread_.wait(kShutdownWaitMilliseconds)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback thread did not stop within 5 seconds.");
        }
        return false;
    }

    commandBus_.reset();

    if (!session_.isNull()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback thread stopped but PlaybackSession was not released.");
        }
        return false;
    }

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

StatePublisher* PlaybackSessionThread::statePublisher() noexcept
{
    return statePublisher_;
}

} // namespace player::playback::application
