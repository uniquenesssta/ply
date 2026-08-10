#pragma once

#include <QSize>
#include <QString>

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>

struct mpv_handle;

namespace player::tools::render_probe {

struct RenderProbeMpvSnapshot final
{
    std::optional<std::int64_t> frameDropCount;
    std::optional<std::int64_t> decoderFrameDropCount;
    std::optional<std::int64_t> mistimedFrameCount;
    std::optional<std::int64_t> delayedFrameCount;
    std::optional<double> containerFps;
    std::optional<double> estimatedVfFps;
    std::optional<double> displayFps;
    std::optional<std::int64_t> videoWidth;
    std::optional<std::int64_t> videoHeight;
    QString hwdecCurrent;
};

struct RenderProbeTimingSummary final
{
    int afterRenderingCount = 0;
    int intervalSampleCount = 0;
    double elapsedMs = 0.0;
    std::optional<double> meanFrameIntervalMs;
    std::optional<double> p95FrameIntervalMs;
    std::optional<double> maxFrameIntervalMs;
};

struct RenderProbePhaseResult final
{
    QString mode;
    QSize logicalWindowSize;
    QSize physicalRenderSize;
    qreal devicePixelRatio = 1.0;
    QSize screenPixelSize;
    qreal screenRefreshRate = 0.0;
    RenderProbeTimingSummary timing;
    RenderProbeMpvSnapshot before;
    RenderProbeMpvSnapshot after;
};

class RenderProbeFrameTimingCollector final
{
public:
    void start();
    void noteFrame() noexcept;
    [[nodiscard]] RenderProbeTimingSummary finish();

private:
    mutable std::mutex mutex_;
    bool active_ = false;
    int frameCount_ = 0;
    std::chrono::steady_clock::time_point startedAt_{};
    std::optional<std::chrono::steady_clock::time_point> lastFrameAt_;
    std::vector<double> frameIntervalsMs_;
};

[[nodiscard]] RenderProbeMpvSnapshot captureRenderProbeMpvSnapshot(mpv_handle* handle) noexcept;

} // namespace player::tools::render_probe
