#include "render_probe_report.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include <optional>

namespace player::tools::render_probe {
namespace {

template <typename T>
QJsonValue optionalNumber(const std::optional<T>& value)
{
    return value.has_value()
        ? QJsonValue(static_cast<double>(*value))
        : QJsonValue(QJsonValue::Null);
}

QJsonValue optionalDelta(
    const std::optional<std::int64_t>& before,
    const std::optional<std::int64_t>& after)
{
    if (!before.has_value() || !after.has_value()) {
        return QJsonValue(QJsonValue::Null);
    }
    return QJsonValue(static_cast<double>(*after - *before));
}

QJsonObject mpvSnapshotObject(const RenderProbeMpvSnapshot& snapshot)
{
    QJsonObject object;
    object.insert(QStringLiteral("frame_drop_count"), optionalNumber(snapshot.frameDropCount));
    object.insert(
        QStringLiteral("decoder_frame_drop_count"),
        optionalNumber(snapshot.decoderFrameDropCount));
    object.insert(
        QStringLiteral("mistimed_frame_count"),
        optionalNumber(snapshot.mistimedFrameCount));
    object.insert(
        QStringLiteral("vo_delayed_frame_count"),
        optionalNumber(snapshot.delayedFrameCount));
    object.insert(QStringLiteral("container_fps"), optionalNumber(snapshot.containerFps));
    object.insert(QStringLiteral("estimated_vf_fps"), optionalNumber(snapshot.estimatedVfFps));
    object.insert(QStringLiteral("display_fps"), optionalNumber(snapshot.displayFps));
    object.insert(QStringLiteral("video_width"), optionalNumber(snapshot.videoWidth));
    object.insert(QStringLiteral("video_height"), optionalNumber(snapshot.videoHeight));
    object.insert(
        QStringLiteral("hwdec_current"),
        snapshot.hwdecCurrent.isEmpty()
            ? QJsonValue(QJsonValue::Null)
            : QJsonValue(snapshot.hwdecCurrent));
    return object;
}

QJsonObject timingObject(const RenderProbeTimingSummary& timing)
{
    QJsonObject object;
    object.insert(QStringLiteral("after_rendering_count"), timing.afterRenderingCount);
    object.insert(QStringLiteral("interval_sample_count"), timing.intervalSampleCount);
    object.insert(QStringLiteral("elapsed_ms"), timing.elapsedMs);
    object.insert(
        QStringLiteral("mean_frame_interval_ms"),
        optionalNumber(timing.meanFrameIntervalMs));
    object.insert(
        QStringLiteral("p95_frame_interval_ms"),
        optionalNumber(timing.p95FrameIntervalMs));
    object.insert(
        QStringLiteral("max_frame_interval_ms"),
        optionalNumber(timing.maxFrameIntervalMs));
    return object;
}

QJsonObject phaseObject(const RenderProbePhaseResult& phase)
{
    QJsonObject window;
    window.insert(QStringLiteral("logical_width"), phase.logicalWindowSize.width());
    window.insert(QStringLiteral("logical_height"), phase.logicalWindowSize.height());
    window.insert(QStringLiteral("device_pixel_ratio"), phase.devicePixelRatio);
    window.insert(QStringLiteral("physical_width"), phase.physicalRenderSize.width());
    window.insert(QStringLiteral("physical_height"), phase.physicalRenderSize.height());

    QJsonObject screen;
    screen.insert(QStringLiteral("pixel_width"), phase.screenPixelSize.width());
    screen.insert(QStringLiteral("pixel_height"), phase.screenPixelSize.height());
    screen.insert(QStringLiteral("refresh_hz"), phase.screenRefreshRate);

    QJsonObject deltas;
    deltas.insert(
        QStringLiteral("frame_drop_count"),
        optionalDelta(phase.before.frameDropCount, phase.after.frameDropCount));
    deltas.insert(
        QStringLiteral("decoder_frame_drop_count"),
        optionalDelta(phase.before.decoderFrameDropCount, phase.after.decoderFrameDropCount));
    deltas.insert(
        QStringLiteral("mistimed_frame_count"),
        optionalDelta(phase.before.mistimedFrameCount, phase.after.mistimedFrameCount));
    deltas.insert(
        QStringLiteral("vo_delayed_frame_count"),
        optionalDelta(phase.before.delayedFrameCount, phase.after.delayedFrameCount));

    QJsonObject object;
    object.insert(QStringLiteral("mode"), phase.mode);
    object.insert(QStringLiteral("window"), window);
    object.insert(QStringLiteral("screen"), screen);
    object.insert(QStringLiteral("qt_render_timing"), timingObject(phase.timing));
    object.insert(QStringLiteral("mpv_before"), mpvSnapshotObject(phase.before));
    object.insert(QStringLiteral("mpv_after"), mpvSnapshotObject(phase.after));
    object.insert(QStringLiteral("mpv_deltas"), deltas);
    return object;
}

QJsonObject environmentObject(const RenderProbeEnvironment& environment)
{
    QJsonObject object;
    object.insert(QStringLiteral("os_product"), environment.osProduct);
    object.insert(QStringLiteral("kernel_type"), environment.kernelType);
    object.insert(QStringLiteral("kernel_version"), environment.kernelVersion);
    object.insert(QStringLiteral("current_cpu_arch"), environment.currentCpuArchitecture);
    object.insert(QStringLiteral("build_cpu_arch"), environment.buildCpuArchitecture);
    object.insert(QStringLiteral("qt_version"), environment.qtVersion);
    object.insert(QStringLiteral("build_type"), environment.buildType);
    object.insert(QStringLiteral("gpu_vendor"), environment.gpuVendor);
    object.insert(QStringLiteral("gpu_renderer"), environment.gpuRenderer);
    object.insert(QStringLiteral("opengl_version"), environment.openGlVersion);
    return object;
}

} // namespace

bool writeRenderProbeReport(
    const RenderProbeReport& report,
    const QString& outputPath,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QJsonArray phases;
    for (const RenderProbePhaseResult& phase : report.phases) {
        phases.append(phaseObject(phase));
    }

    QJsonObject root;
    root.insert(QStringLiteral("schema"), QStringLiteral("player-r4-render-baseline-v1"));
    root.insert(QStringLiteral("label"), report.label);
    root.insert(QStringLiteral("media_source"), report.mediaSource);
    root.insert(
        QStringLiteral("captured_at_utc"),
        report.capturedAtUtc.toUTC().toString(Qt::ISODateWithMs));
    root.insert(QStringLiteral("environment"), environmentObject(report.environment));
    root.insert(QStringLiteral("phases"), phases);

    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
    QTextStream standardOutput(stdout);
    standardOutput << QString::fromUtf8(json);
    standardOutput.flush();

    if (outputPath.isEmpty()) {
        return true;
    }

    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to write render probe report '%1': %2")
                                .arg(outputPath, file.errorString());
        }
        return false;
    }

    if (file.write(json) != static_cast<qint64>(json.size())) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to write the complete render probe report '%1'.")
                                .arg(outputPath);
        }
        return false;
    }

    return true;
}

} // namespace player::tools::render_probe
