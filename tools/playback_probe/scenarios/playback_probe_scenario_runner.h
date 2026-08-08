#pragma once

#include "playback_probe_scenario.h"
#include "runtime/playback_probe_runtime.h"
#include "trace/playback_probe_trace.h"

#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QtGlobal>

namespace player::tools::playback_probe {

class PlaybackProbeScenarioRunner final : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackProbeScenarioRunner(QObject* parent = nullptr);

    PlaybackProbeScenarioRunner(const PlaybackProbeScenarioRunner&) = delete;
    PlaybackProbeScenarioRunner& operator=(const PlaybackProbeScenarioRunner&) = delete;

    [[nodiscard]] bool start(
        const PlaybackProbeScenario& scenario,
        QString* errorMessage = nullptr);

signals:
    void finished(bool success, const QString& diagnostic);

private slots:
    void handleEvent(const player::playback::mpv::MpvEvent& event);
    void handleStepTimeout();

private:
    void advance();
    void armTimeout(const QString& stepLabel);
    void fail(const QString& diagnostic);
    void beginFinalize(bool success, const QString& diagnostic);

    [[nodiscard]] bool takeCommandReply(quint64 requestId);
    [[nodiscard]] bool takeEvent(const ProbeWaitEventStep& step);
    [[nodiscard]] bool takeFlagProperty(const ProbeWaitFlagPropertyStep& step);
    [[nodiscard]] bool takeStringProperty(const ProbeWaitStringPropertyStep& step);
    [[nodiscard]] bool scenarioExpectsEndReason(
        player::playback::mpv::MpvEndFileReason reason) const;

    static constexpr int kStepTimeoutMs = 10000;
    static constexpr qsizetype kMaximumPendingEvents = 512;

    PlaybackProbeRuntime runtime_;
    PlaybackProbeTrace trace_;
    PlaybackProbeScenario scenario_;
    QList<player::playback::mpv::MpvEvent> pendingEvents_;
    QSet<quint64> submittedRequestIds_;
    QTimer stepTimer_;
    QString currentStep_;
    qsizetype stepIndex_ = 0;
    bool running_ = false;
    bool finalizing_ = false;
};

} // namespace player::tools::playback_probe
