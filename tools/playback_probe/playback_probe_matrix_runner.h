#pragma once

#include "fixtures/playback_probe_media_set.h"
#include "scenarios/playback_probe_scenario.h"

#include <QList>
#include <QObject>
#include <QString>

namespace player::tools::playback_probe {

class PlaybackProbeScenarioRunner;

class PlaybackProbeMatrixRunner final : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackProbeMatrixRunner(QObject* parent = nullptr);

    PlaybackProbeMatrixRunner(const PlaybackProbeMatrixRunner&) = delete;
    PlaybackProbeMatrixRunner& operator=(const PlaybackProbeMatrixRunner&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);

signals:
    void finished(int exitCode);

private slots:
    void handleScenarioFinished(bool success, const QString& diagnostic);

private:
    void startNextScenario();
    void log(const QString& message) const;
    void fail(const QString& diagnostic);

    PlaybackProbeMediaSet mediaSet_;
    QList<PlaybackProbeScenario> scenarios_;
    PlaybackProbeScenarioRunner* activeScenario_ = nullptr;
    qsizetype scenarioIndex_ = 0;
    bool started_ = false;
    bool finished_ = false;
};

} // namespace player::tools::playback_probe
