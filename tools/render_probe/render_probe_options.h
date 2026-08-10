#pragma once

#include <QSize>
#include <QString>
#include <QStringList>

#include <optional>

namespace player::tools::render_probe {

struct RenderProbeOptions final
{
    QString mediaSource;
    QString label;
    QString outputPath;
    QSize windowSize{1280, 720};
    int phaseDurationMs = 10'000;
    bool showHelp = false;
};

class RenderProbeOptionsParser final
{
public:
    [[nodiscard]] static std::optional<RenderProbeOptions> parse(
        const QStringList& arguments,
        QString* errorMessage = nullptr);
    [[nodiscard]] static QString usage();
};

} // namespace player::tools::render_probe
