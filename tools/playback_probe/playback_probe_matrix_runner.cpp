#include "playback_probe_matrix_runner.h"

#include "scenarios/playback_probe_scenario_catalog.h"
#include "scenarios/playback_probe_scenario_runner.h"

#include <QTextStream>
#include <QTimer>

namespace player::tools::playback_probe {

PlaybackProbeMatrixRunner::PlaybackProbeMatrixRunner(QObject* parent)
    : QObject(parent)
{
}

bool PlaybackProbeMatrixRunner::start(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (started_) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback probe matrix can only be started once.");
        }
        return false;
    }

    QString error;
    if (!mediaSet_.create(&error)) {
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    scenarios_ = PlaybackProbeScenarioCatalog::create(mediaSet_);
    if (scenarios_.size() != 8) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback probe matrix must contain exactly 8 R2-11 scenarios.");
        }
        return false;
    }

    started_ = true;
    scenarioIndex_ = 0;
    log(QStringLiteral("matrix initialized with 8 generated-media scenarios"));
    QTimer::singleShot(0, this, &PlaybackProbeMatrixRunner::startNextScenario);
    return true;
}

void PlaybackProbeMatrixRunner::handleScenarioFinished(
    bool success,
    const QString& diagnostic)
{
    if (finished_ || activeScenario_ == nullptr || scenarioIndex_ >= scenarios_.size()) {
        return;
    }

    const QString scenarioId = scenarios_.at(scenarioIndex_).id;
    activeScenario_->deleteLater();
    activeScenario_ = nullptr;

    if (!success) {
        fail(QStringLiteral("scenario '%1' failed: %2")
                 .arg(scenarioId, diagnostic.isEmpty() ? QStringLiteral("no diagnostic") : diagnostic));
        return;
    }

    log(QStringLiteral("scenario PASS [%1/8]: %2")
            .arg(scenarioIndex_ + 1)
            .arg(scenarioId));
    ++scenarioIndex_;
    QTimer::singleShot(0, this, &PlaybackProbeMatrixRunner::startNextScenario);
}

void PlaybackProbeMatrixRunner::startNextScenario()
{
    if (finished_) {
        return;
    }

    if (scenarioIndex_ >= scenarios_.size()) {
        finished_ = true;
        log(QStringLiteral("MATRIX PASS: 8/8 scripted headless scenarios"));
        emit finished(0);
        return;
    }

    const PlaybackProbeScenario& scenario = scenarios_.at(scenarioIndex_);
    log(QStringLiteral("scenario START [%1/8]: %2 — %3")
            .arg(scenarioIndex_ + 1)
            .arg(scenario.id, scenario.description));

    activeScenario_ = new PlaybackProbeScenarioRunner(this);
    connect(
        activeScenario_,
        &PlaybackProbeScenarioRunner::finished,
        this,
        &PlaybackProbeMatrixRunner::handleScenarioFinished);

    QString error;
    if (!activeScenario_->start(scenario, &error)) {
        activeScenario_->deleteLater();
        activeScenario_ = nullptr;
        fail(QStringLiteral("unable to start scenario '%1': %2").arg(scenario.id, error));
    }
}

void PlaybackProbeMatrixRunner::log(const QString& message) const
{
    QTextStream output(stdout);
    output << "[playback_probe] " << message << Qt::endl;
}

void PlaybackProbeMatrixRunner::fail(const QString& diagnostic)
{
    if (finished_) {
        return;
    }

    finished_ = true;
    QTextStream error(stderr);
    error << "[playback_probe] MATRIX FAIL: " << diagnostic << Qt::endl;
    emit finished(2);
}

} // namespace player::tools::playback_probe
