#include "playback_session_test_harness.h"

#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/session/playback_session.h"

#include <QElapsedTimer>
#include <QMetaObject>
#include <QMutexLocker>

namespace player::playback::application::test_support {

PlaybackSessionTestHarness::PlaybackSessionTestHarness()
{
    thread_.setObjectName(QStringLiteral("PlaybackSessionTestThread"));
    session_ = new PlaybackSession();
    session_->moveToThread(&thread_);
    bus_ = std::make_unique<PlaybackCommandBus>(*session_);

    QObject::connect(
        &thread_,
        &QThread::finished,
        session_,
        &QObject::deleteLater);
    QObject::connect(
        session_,
        &PlaybackSession::ready,
        session_,
        [this]() {
            readyThread_.store(QThread::currentThread(), std::memory_order_release);
            ready_.release();
        },
        Qt::DirectConnection);
    QObject::connect(
        session_,
        &PlaybackSession::startupFailed,
        session_,
        [this](const QString& diagnostic) {
            {
                QMutexLocker lock(&diagnosticMutex_);
                startupDiagnostic_ = diagnostic;
            }
            startupFailed_.release();
        },
        Qt::DirectConnection);
    QObject::connect(
        session_,
        &PlaybackSession::snapshotCommitted,
        session_,
        [this](const player::playback::domain::PlaybackSnapshot& snapshot) {
            observeSnapshot(snapshot);
        },
        Qt::DirectConnection);
    QObject::connect(
        session_,
        &PlaybackSession::invariantViolationDetected,
        session_,
        [this](int count) {
            invariantViolations_.fetch_add(count, std::memory_order_relaxed);
        },
        Qt::DirectConnection);
    QObject::connect(
        session_,
        &PlaybackSession::stopped,
        session_,
        [this]() {
            stopped_.release();
        },
        Qt::DirectConnection);
}

PlaybackSessionTestHarness::~PlaybackSessionTestHarness()
{
    QString ignored;
    if (!stop(&ignored) && thread_.isRunning()) {
        thread_.quit();
        thread_.wait();
    }
}

bool PlaybackSessionTestHarness::start(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    thread_.start();
    if (!QMetaObject::invokeMethod(session_.data(), "initialize", Qt::QueuedConnection)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to queue test PlaybackSession initialization.");
        }
        return false;
    }

    if (ready_.tryAcquire(1, 5000)) {
        return true;
    }

    if (startupFailed_.tryAcquire(1)) {
        if (errorMessage != nullptr) {
            QMutexLocker lock(&diagnosticMutex_);
            *errorMessage = startupDiagnostic_;
        }
    } else if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Timed out waiting for PlaybackSession initialization.");
    }
    return false;
}

bool PlaybackSessionTestHarness::stop(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    if (!thread_.isRunning()) {
        bus_.reset();
        return true;
    }

    if (!session_.isNull()) {
        if (!QMetaObject::invokeMethod(session_.data(), "shutdown", Qt::QueuedConnection)) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Failed to queue test PlaybackSession shutdown.");
            }
            return false;
        }
        if (!stopped_.tryAcquire(1, 5000)) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Timed out waiting for PlaybackSession shutdown.");
            }
            return false;
        }
    }

    thread_.quit();
    if (!thread_.wait(5000)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("PlaybackSession test thread did not stop.");
        }
        return false;
    }

    bus_.reset();
    return true;
}

PlaybackCommandBus& PlaybackSessionTestHarness::bus() const
{
    return *bus_;
}

quint64 PlaybackSessionTestHarness::snapshotSequence() const
{
    QMutexLocker lock(&snapshotMutex_);
    return snapshotSequence_;
}

player::playback::domain::PlaybackSnapshot PlaybackSessionTestHarness::latestSnapshot() const
{
    QMutexLocker lock(&snapshotMutex_);
    return snapshot_;
}

QThread* PlaybackSessionTestHarness::snapshotCallbackThread() const
{
    QMutexLocker lock(&snapshotMutex_);
    return snapshotCallbackThread_;
}

bool PlaybackSessionTestHarness::waitForSnapshotAfter(
    quint64 previousSequence,
    const std::function<bool(const player::playback::domain::PlaybackSnapshot&)>& predicate,
    int timeoutMilliseconds)
{
    QElapsedTimer timer;
    timer.start();

    QMutexLocker lock(&snapshotMutex_);
    while (snapshotSequence_ <= previousSequence || !predicate(snapshot_)) {
        const qint64 remaining = timeoutMilliseconds - timer.elapsed();
        if (remaining <= 0) {
            return false;
        }
        if (!snapshotChanged_.wait(&snapshotMutex_, static_cast<unsigned long>(remaining))) {
            return snapshotSequence_ > previousSequence && predicate(snapshot_);
        }
    }
    return true;
}

QThread* PlaybackSessionTestHarness::playbackThread() noexcept
{
    return &thread_;
}

QThread* PlaybackSessionTestHarness::readyThread() const noexcept
{
    return readyThread_.load(std::memory_order_acquire);
}

int PlaybackSessionTestHarness::invariantViolations() const noexcept
{
    return invariantViolations_.load(std::memory_order_relaxed);
}

void PlaybackSessionTestHarness::observeSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    QMutexLocker lock(&snapshotMutex_);
    snapshot_ = snapshot;
    snapshotCallbackThread_ = QThread::currentThread();
    ++snapshotSequence_;
    snapshotChanged_.wakeAll();
}

} // namespace player::playback::application::test_support
