#pragma once

#include "playback/infrastructure/mpv/commands/mpv_command_request.h"
#include "playback/infrastructure/mpv/events/mpv_event.h"
#include "runtime/playback_probe_runtime.h"

#include <QObject>
#include <QString>
#include <QTimer>
#include <QtGlobal>

namespace player::tools::playback_probe {

class PlaybackProbeRunner final : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackProbeRunner(QObject* parent = nullptr);

    PlaybackProbeRunner(const PlaybackProbeRunner&) = delete;
    PlaybackProbeRunner& operator=(const PlaybackProbeRunner&) = delete;

    [[nodiscard]] bool start(const QString& source, QString* errorMessage = nullptr);

signals:
    void finished(int exitCode);

private slots:
    void handleEvent(const player::playback::mpv::MpvEvent& event);
    void handleStepTimeout();

private:
    enum class Phase
    {
        Idle,
        Loading,
        Pausing,
        WaitingForFileLoaded,
        Playing,
        Seeking,
        Stopping,
        WaitingForEndFile,
        Finished,
    };

    [[nodiscard]] bool submit(
        quint64 requestId,
        const player::playback::mpv::MpvCommandRequest& request,
        const QString& stepName,
        Phase phase,
        QString* errorMessage = nullptr);
    [[nodiscard]] bool submitPause(QString* errorMessage = nullptr);
    [[nodiscard]] bool submitPlay(QString* errorMessage = nullptr);
    [[nodiscard]] bool submitSeek(QString* errorMessage = nullptr);
    [[nodiscard]] bool submitStop(QString* errorMessage = nullptr);

    void armTimeout(const QString& stepName);
    void log(const QString& message) const;
    void fail(const QString& message, int exitCode = 2);
    void succeed();
    void finalize(int exitCode);

    PlaybackProbeRuntime runtime_;
    QTimer stepTimer_;
    QString currentStep_;
    Phase phase_ = Phase::Idle;
    bool fileLoaded_ = false;
    bool stopReplyReceived_ = false;
    bool endFileReceived_ = false;
    bool finished_ = false;
};

} // namespace player::tools::playback_probe
