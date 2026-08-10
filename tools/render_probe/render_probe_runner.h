#pragma once

#include "render_probe_environment.h"
#include "render_probe_metrics.h"
#include "render_probe_options.h"

#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QObject>
#include <QElapsedTimer>
#include <QQuickWindow>
#include <QTimer>

#include <atomic>
#include <memory>

namespace player::playback::mpv {
class MpvHandle;
}

namespace player::tools::render_probe {

class RenderProbeRunner final : public QObject
{
    Q_OBJECT

public:
    explicit RenderProbeRunner(RenderProbeOptions options, QObject* parent = nullptr);
    ~RenderProbeRunner() override;

    RenderProbeRunner(const RenderProbeRunner&) = delete;
    RenderProbeRunner& operator=(const RenderProbeRunner&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);

signals:
    void finished(int exitCode);

private:
    enum class Stage
    {
        Idle,
        WaitingForWindow,
        WaitingForMedia,
        WaitingForFullscreen,
        MeasuringWindowed,
        MeasuringFullscreen,
        ShuttingDown,
        Finished,
    };

    void pollStage();
    void loadMedia();
    void settleAndStartWindowed();
    void startWindowedMeasurement();
    void startFullscreenTransition();
    void startFullscreenMeasurement();
    void finishMeasurement(const QString& mode);
    void beginShutdown();
    void completeShutdown();
    void fail(QString message);
    void finish(int exitCode);
    void updateVideoItemSize();

    [[nodiscard]] bool windowReady() const noexcept;
    [[nodiscard]] bool mediaReady() noexcept;
    [[nodiscard]] bool fullscreenReady() const noexcept;
    [[nodiscard]] RenderProbePhaseResult makePhaseResult(
        const QString& mode,
        RenderProbeMpvSnapshot before,
        RenderProbeMpvSnapshot after,
        RenderProbeTimingSummary timing) const;

    static constexpr int kPollIntervalMs = 50;
    static constexpr int kStartupTimeoutMs = 10'000;
    static constexpr int kModeSettleMs = 1'500;
    static constexpr int kShutdownTimeoutMs = 10'000;
    static constexpr int kRequiredRenderedFramesAfterPipelineReady = 3;

    RenderProbeOptions options_;
    RenderProbeEnvironment environment_;
    Stage stage_ = Stage::Idle;
    QString pendingError_;
    QElapsedTimer stageElapsed_;
    QTimer pollTimer_;
    QTimer phaseTimer_;
    QQuickWindow window_;
    player::playback::infrastructure::mpv::render::MpvVideoItem videoItem_;
    std::unique_ptr<player::playback::mpv::MpvHandle> core_;
    RenderProbeFrameTimingCollector frameTiming_;
    RenderProbeMpvSnapshot phaseBefore_;
    QVector<RenderProbePhaseResult> phases_;
    std::atomic_int renderedFrameCount_{0};
    int minimumRenderedFrameCountForMediaReady_ = -1;
};

} // namespace player::tools::render_probe
