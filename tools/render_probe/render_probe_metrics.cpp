#include "render_probe_metrics.h"

#include <mpv/client.h>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace player::tools::render_probe {
namespace {

std::optional<std::int64_t> readInt64(mpv_handle* handle, const char* name) noexcept
{
    if (handle == nullptr || name == nullptr) {
        return std::nullopt;
    }

    std::int64_t value = 0;
    if (mpv_get_property(handle, name, MPV_FORMAT_INT64, &value) < 0) {
        return std::nullopt;
    }
    return value;
}

std::optional<double> readDouble(mpv_handle* handle, const char* name) noexcept
{
    if (handle == nullptr || name == nullptr) {
        return std::nullopt;
    }

    double value = 0.0;
    if (mpv_get_property(handle, name, MPV_FORMAT_DOUBLE, &value) < 0
        || !std::isfinite(value)) {
        return std::nullopt;
    }
    return value;
}

QString readString(mpv_handle* handle, const char* name) noexcept
{
    if (handle == nullptr || name == nullptr) {
        return {};
    }

    char* value = mpv_get_property_string(handle, name);
    if (value == nullptr) {
        return {};
    }

    const QString result = QString::fromUtf8(value);
    mpv_free(value);
    return result;
}

std::optional<double> percentile95(std::vector<double> values)
{
    if (values.empty()) {
        return std::nullopt;
    }

    std::sort(values.begin(), values.end());
    const double position = 0.95 * static_cast<double>(values.size() - 1);
    const auto lowerIndex = static_cast<std::size_t>(std::floor(position));
    const auto upperIndex = static_cast<std::size_t>(std::ceil(position));
    if (lowerIndex == upperIndex) {
        return values.at(lowerIndex);
    }

    const double weight = position - static_cast<double>(lowerIndex);
    return values.at(lowerIndex) * (1.0 - weight) + values.at(upperIndex) * weight;
}

} // namespace

void RenderProbeFrameTimingCollector::start()
{
    std::scoped_lock lock(mutex_);
    active_ = true;
    frameCount_ = 0;
    startedAt_ = std::chrono::steady_clock::now();
    lastFrameAt_.reset();
    frameIntervalsMs_.clear();
}

void RenderProbeFrameTimingCollector::noteFrame() noexcept
{
    const auto now = std::chrono::steady_clock::now();

    std::scoped_lock lock(mutex_);
    if (!active_) {
        return;
    }

    ++frameCount_;
    if (lastFrameAt_.has_value()) {
        const auto elapsed = std::chrono::duration<double, std::milli>(now - *lastFrameAt_);
        frameIntervalsMs_.push_back(elapsed.count());
    }
    lastFrameAt_ = now;
}

RenderProbeTimingSummary RenderProbeFrameTimingCollector::finish()
{
    const auto finishedAt = std::chrono::steady_clock::now();

    std::scoped_lock lock(mutex_);
    active_ = false;

    RenderProbeTimingSummary summary;
    summary.afterRenderingCount = frameCount_;
    summary.intervalSampleCount = static_cast<int>(frameIntervalsMs_.size());
    summary.elapsedMs = std::chrono::duration<double, std::milli>(finishedAt - startedAt_).count();

    if (!frameIntervalsMs_.empty()) {
        const double total = std::accumulate(frameIntervalsMs_.begin(), frameIntervalsMs_.end(), 0.0);
        summary.meanFrameIntervalMs = total / static_cast<double>(frameIntervalsMs_.size());
        summary.p95FrameIntervalMs = percentile95(frameIntervalsMs_);
        summary.maxFrameIntervalMs = *std::max_element(
            frameIntervalsMs_.begin(),
            frameIntervalsMs_.end());
    }

    return summary;
}

RenderProbeMpvSnapshot captureRenderProbeMpvSnapshot(mpv_handle* handle) noexcept
{
    RenderProbeMpvSnapshot snapshot;
    snapshot.frameDropCount = readInt64(handle, "frame-drop-count");
    snapshot.decoderFrameDropCount = readInt64(handle, "decoder-frame-drop-count");
    snapshot.mistimedFrameCount = readInt64(handle, "mistimed-frame-count");
    snapshot.delayedFrameCount = readInt64(handle, "vo-delayed-frame-count");
    snapshot.containerFps = readDouble(handle, "container-fps");
    snapshot.estimatedVfFps = readDouble(handle, "estimated-vf-fps");
    snapshot.displayFps = readDouble(handle, "display-fps");
    snapshot.videoWidth = readInt64(handle, "video-params/w");
    snapshot.videoHeight = readInt64(handle, "video-params/h");
    snapshot.hwdecCurrent = readString(handle, "hwdec-current");
    return snapshot;
}

} // namespace player::tools::render_probe
