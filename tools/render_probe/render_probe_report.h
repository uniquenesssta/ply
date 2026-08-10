#pragma once

#include "render_probe_environment.h"
#include "render_probe_metrics.h"

#include <QDateTime>
#include <QString>
#include <QVector>

namespace player::tools::render_probe {

struct RenderProbeReport final
{
    QString label;
    QString mediaSource;
    QDateTime capturedAtUtc;
    RenderProbeEnvironment environment;
    QVector<RenderProbePhaseResult> phases;
};

[[nodiscard]] bool writeRenderProbeReport(
    const RenderProbeReport& report,
    const QString& outputPath,
    QString* errorMessage = nullptr);

} // namespace player::tools::render_probe
