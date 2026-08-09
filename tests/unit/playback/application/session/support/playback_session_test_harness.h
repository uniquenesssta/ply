#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QMutex>
#include <QPointer>
#include <QSemaphore>
#include <QString>
#include <QThread>
#include <QWaitCondition>

#include <atomic>
#include <functional>
#include <memory>

namespace player::playback::application {

class PlaybackCommandBus;
class PlaybackSession;

namespace test_support {

class PlaybackSessionTestHarness final
{
public:
    PlaybackSessionTestHarness();
    ~PlaybackSessionTestHarness();

    PlaybackSessionTestHarness(const PlaybackSessionTestHarness&) = delete;
    PlaybackSessionTestHarness& operator=(const PlaybackSessionTestHarness&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);
    [[nodiscard]] bool stop(QString* errorMessage = nullptr);

    [[nodiscard]] PlaybackCommandBus& bus() const;
    [[nodiscard]] quint64 snapshotSequence() const;
    [[nodiscard]] player::playback::domain::PlaybackSnapshot latestSnapshot() const;
    [[nodiscard]] QThread* snapshotCallbackThread() const;
    [[nodiscard]] bool waitForSnapshotAfter(
        quint64 previousSequence,
        const std::function<bool(const player::playback::domain::PlaybackSnapshot&)>& predicate,
        int timeoutMilliseconds);

    [[nodiscard]] QThread* playbackThread() noexcept;
    [[nodiscard]] QThread* readyThread() const noexcept;
    [[nodiscard]] int invariantViolations() const noexcept;

private:
    void observeSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

    QThread thread_;
    QPointer<PlaybackSession> session_;
    std::unique_ptr<PlaybackCommandBus> bus_;

    mutable QMutex snapshotMutex_;
    QWaitCondition snapshotChanged_;
    player::playback::domain::PlaybackSnapshot snapshot_;
    QThread* snapshotCallbackThread_ = nullptr;
    quint64 snapshotSequence_ = 0;

    QSemaphore ready_;
    QSemaphore startupFailed_;
    QSemaphore stopped_;
    mutable QMutex diagnosticMutex_;
    QString startupDiagnostic_;
    std::atomic<QThread*> readyThread_{nullptr};
    std::atomic<int> invariantViolations_{0};
};

} // namespace test_support
} // namespace player::playback::application
