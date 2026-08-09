#pragma once

#include "playback/application/requests/request_tracker.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/events/playback_event.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QObject>
#include <QString>
#include <QtGlobal>

#include <memory>

class QTimer;

namespace player::playback::application {

class PlaybackCommandBus;
class PlaybackSessionBackend;
class PlaybackSessionThread;

class PlaybackSession final : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackSession(QObject* parent = nullptr);
    ~PlaybackSession() override;

    PlaybackSession(const PlaybackSession&) = delete;
    PlaybackSession& operator=(const PlaybackSession&) = delete;

signals:
    void ready();
    void startupFailed(const QString& diagnostic);
    void snapshotCommitted(const player::playback::domain::PlaybackSnapshot& snapshot);
    void invariantViolationDetected(int violationCount);
    void stopped();

private slots:
    void initialize();
    void shutdown();

private:
    friend class PlaybackCommandBus;
    friend class PlaybackSessionThread;

    void processCommand(player::playback::domain::PlaybackCommand command);
    void handleBackendEvent(const player::playback::domain::PlaybackEvent& event);
    void handleCommandReply(const player::playback::domain::CommandReplyEvent& reply);
    void beginMediaLoad(const player::playback::domain::PlaybackCommand& command);
    void submitTrackedCommand(const player::playback::domain::PlaybackCommand& command);
    void commitSnapshot(player::playback::domain::PlaybackSnapshot next);
    void commitSubmissionFailure(
        const player::playback::domain::PlaybackCommand& command,
        QString diagnostic);
    void commitTrackingFailure(RequestTrackStatus status);
    void ensureRequestTimeoutTimer();

    [[nodiscard]] bool isOnOwningThread() const noexcept;
    [[nodiscard]] player::playback::domain::MediaGeneration allocateMediaGeneration() noexcept;

    std::unique_ptr<PlaybackSessionBackend> backend_;
    RequestTracker requestTracker_;
    QTimer* requestTimeoutTimer_ = nullptr;
    player::playback::domain::PlaybackSnapshot snapshot_;
    quint64 nextGenerationValue_ = 1;
    bool initialized_ = false;
    bool stopping_ = false;
};

} // namespace player::playback::application
