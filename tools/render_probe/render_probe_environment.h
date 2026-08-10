#pragma once

#include <QString>

namespace player::tools::render_probe {

struct RenderProbeEnvironment final
{
    QString osProduct;
    QString kernelType;
    QString kernelVersion;
    QString currentCpuArchitecture;
    QString buildCpuArchitecture;
    QString qtVersion;
    QString buildType;
    QString gpuVendor;
    QString gpuRenderer;
    QString openGlVersion;
};

[[nodiscard]] RenderProbeEnvironment captureRenderProbeEnvironment(
    QString* warningMessage = nullptr);

} // namespace player::tools::render_probe
