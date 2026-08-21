#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QThread>
#include <QtGlobal>

#include <memory>

namespace player::playback::application {

class PlaybackCommandBus;
class PlaybackSession;
class StatePublisher;

class PlaybackSessionThread final : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackSessionThread(QObject* parent = nullptr);
    ~PlaybackSessionThread() override;

    PlaybackSessionThread(const PlaybackSessionThread&) = delete;
    PlaybackSessionThread& operator=(const PlaybackSessionThread&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);
    [[nodiscard]] bool stop(QString* errorMessage = nullptr);

    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] PlaybackCommandBus* commandBus() noexcept;
    [[nodiscard]] StatePublisher* statePublisher() noexcept;

signals:
    void ready();
    void renderCoreReady(quintptr nativeHandle);
    void startupFailed(const QString& diagnostic);
    void requestFailed(quint8 requestType, const QString& diagnostic);
    void requestFinished(
        quint8 requestType,
        quint64 requestId,
        quint64 generation,
        bool succeeded,
        const QString& diagnostic);
    void stopped();

private:
    QThread thread_;
    QPointer<PlaybackSession> session_;
    std::unique_ptr<PlaybackCommandBus> commandBus_;
    StatePublisher* statePublisher_ = nullptr;
};

} // namespace player::playback::application
