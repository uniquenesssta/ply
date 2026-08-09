#include "fixtures/session_test_media.h"

#include "foundation/ids/request_id.h"
#include "playback/application/command_bus/playback_command_bus.h"
#include "playback/application/session/playback_session.h"
#include "playback/application/session/playback_session_thread.h"
#include "playback/domain/commands/playback_command.h"

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <QElapsedTimer>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QSemaphore>
#include <QTemporaryDir>
#include <QThread>
#include <QWaitCondition>

#include <atomic>
#include <cmath>
#include <functional>
#include <memory>
#include <utility>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;

PlaybackCommand makeCommand(quint64 requestId, PlaybackCommandPayload payload)
{
    return PlaybackCommand{player::ids::RequestId{requestId}, std::move(payload)};
}

class SnapshotLatch final
{
public:
    void observe(const PlaybackSnapshot& snapshot)
    {
        QMutexLocker lock(&mutex_);
        snapshot_ = snapshot;
        callbackThread_ = QThread::currentThread();
        ++sequence_;
        changed_.wakeAll();
    }

    [[nodiscard]] quint64 sequence() const
    {
        QMutexLocker lock(&mutex_);
        return sequence_;
    }

    [[nodiscard]] PlaybackSnapshot snapshot() const
    {
        QMutexLocker lock(&mutex_);
        return snapshot_;
    }

    [[nodiscard]] QThread* callbackThread() const
    {
        QMutexLocker lock(&mutex_);
        return callbackThread_;
    }

    [[nodiscard]] bool waitForAfter(
        quint64 previousSequence,
        const std::function<bool(const PlaybackSnapshot&)>& predicate,
        int timeoutMilliseconds)
    {
        QElapsedTimer timer;
        timer.start();

        QMutexLocker lock(&mutex_);
        while (sequence_ <= previousSequence || !predicate(snapshot_)) {
            const qint64 remaining = timeoutMilliseconds - timer.elapsed();
            if (remaining <= 0) {
                return false;
            }
            if (!changed_.wait(&mutex_, static_cast<unsigned long>(remaining))) {
                return sequence_ > previousSequence && predicate(snapshot_);
            }
        }
        return true;
    }

private:
    mutable QMutex mutex_;
    QWaitCondition changed_;
    PlaybackSnapshot snapshot_;
    QThread* callbackThread_ = nullptr;
    quint64 sequence_ = 0;
};

class SessionHarness final
{
public:
    SessionHarness()
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
            [this](const PlaybackSnapshot& snapshot) {
                snapshots_.observe(snapshot);
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

    ~SessionHarness()
    {
        QString ignored;
        if (!stop(&ignored) && thread_.isRunning()) {
            thread_.quit();
            thread_.wait();
        }
    }

    [[nodiscard]] bool start(QString* errorMessage)
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

    [[nodiscard]] bool stop(QString* errorMessage)
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

    [[nodiscard]] PlaybackCommandBus& bus() const
    {
        return *bus_;
    }

    [[nodiscard]] SnapshotLatch& snapshots() noexcept
    {
        return snapshots_;
    }

    [[nodiscard]] QThread* playbackThread() noexcept
    {
        return &thread_;
    }

    [[nodiscard]] QThread* readyThread() const noexcept
    {
        return readyThread_.load(std::memory_order_acquire);
    }

    [[nodiscard]] int invariantViolations() const noexcept
    {
        return invariantViolations_.load(std::memory_order_relaxed);
    }

private:
    QThread thread_;
    QPointer<PlaybackSession> session_;
    std::unique_ptr<PlaybackCommandBus> bus_;
    SnapshotLatch snapshots_;
    QSemaphore ready_;
    QSemaphore startupFailed_;
    QSemaphore stopped_;
    mutable QMutex diagnosticMutex_;
    QString startupDiagnostic_;
    std::atomic<QThread*> readyThread_{nullptr};
    std::atomic<int> invariantViolations_{0};
};

} // namespace

class PlaybackSessionTest final : public QObject
{
    Q_OBJECT

private slots:
    void realMediaMainPathRunsOnPlaybackThread();
    void threadHostStartsAndStops();
};

void PlaybackSessionTest::realMediaMainPathRunsOnPlaybackThread()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString mediaPath = directory.filePath(QStringLiteral("session-main-path.wav"));
    QString error;
    QVERIFY2(
        test_support::writeSilentPcmWav(mediaPath, 3000, &error),
        qPrintable(error));

    SessionHarness harness;
    QVERIFY2(harness.start(&error), qPrintable(error));
    QCOMPARE(harness.readyThread(), harness.playbackThread());

    quint64 requestId = 1;

    quint64 sequence = harness.snapshots().sequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, LoadMediaCommand{mediaPath}),
            &error),
        qPrintable(error));
    QVERIFY(harness.snapshots().waitForAfter(
        sequence,
        [&mediaPath](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.generation().value() == 1
                && snapshot.media().source.has_value()
                && *snapshot.media().source == mediaPath;
        },
        7000));

    sequence = harness.snapshots().sequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, TransportCommand{TransportAction::Pause}),
            &error),
        qPrintable(error));
    QVERIFY(harness.snapshots().waitForAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Paused;
        },
        5000));

    sequence = harness.snapshots().sequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, TransportCommand{TransportAction::Play}),
            &error),
        qPrintable(error));
    QVERIFY(harness.snapshots().waitForAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Playing;
        },
        5000));

    sequence = harness.snapshots().sequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, TransportCommand{TransportAction::Pause}),
            &error),
        qPrintable(error));
    QVERIFY(harness.snapshots().waitForAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Ready
                && snapshot.transport() == PlaybackTransportState::Paused;
        },
        5000));

    sequence = harness.snapshots().sequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, SeekCommand{0.75, SeekMode::Absolute}),
            &error),
        qPrintable(error));
    QVERIFY(harness.snapshots().waitForAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.timeline().positionSeconds.has_value()
                && std::abs(*snapshot.timeline().positionSeconds - 0.75) < 0.35;
        },
        5000));

    sequence = harness.snapshots().sequence();
    QVERIFY2(
        harness.bus().submit(
            makeCommand(requestId++, TransportCommand{TransportAction::Stop}),
            &error),
        qPrintable(error));
    QVERIFY(harness.snapshots().waitForAfter(
        sequence,
        [](const PlaybackSnapshot& snapshot) {
            return snapshot.lifecycle() == PlaybackLifecycleState::Empty
                && snapshot.transport() == PlaybackTransportState::Stopped
                && snapshot.generation().value() == 1
                && !snapshot.media().source.has_value();
        },
        5000));

    QCOMPARE(harness.snapshots().callbackThread(), harness.playbackThread());
    QCOMPARE(harness.invariantViolations(), 0);
    QVERIFY2(harness.stop(&error), qPrintable(error));
}

void PlaybackSessionTest::threadHostStartsAndStops()
{
    PlaybackSessionThread host;
    QSignalSpy readySpy(&host, &PlaybackSessionThread::ready);
    QSignalSpy startupFailureSpy(&host, &PlaybackSessionThread::startupFailed);
    QSignalSpy stoppedSpy(&host, &PlaybackSessionThread::stopped);

    QString error;
    QVERIFY2(host.start(&error), qPrintable(error));
    QTRY_COMPARE_WITH_TIMEOUT(readySpy.count(), 1, 5000);
    QCOMPARE(startupFailureSpy.count(), 0);
    QVERIFY(host.isRunning());
    QVERIFY(host.commandBus() != nullptr);

    QVERIFY2(host.stop(&error), qPrintable(error));
    QVERIFY(!host.isRunning());
    QTRY_VERIFY_WITH_TIMEOUT(stoppedSpy.count() >= 1, 1000);
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::PlaybackSessionTest)
#include "playback_session_test.moc"
