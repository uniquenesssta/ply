#include "render_probe_options.h"

#include <QRegularExpression>

namespace player::tools::render_probe {
namespace {

void assignError(QString* errorMessage, const QString& message)
{
    if (errorMessage != nullptr) {
        *errorMessage = message;
    }
}

bool takeValue(
    const QStringList& arguments,
    qsizetype* index,
    QString* value,
    QString* errorMessage)
{
    if (*index + 1 >= arguments.size()) {
        assignError(
            errorMessage,
            QStringLiteral("Missing value after '%1'.").arg(arguments.at(*index)));
        return false;
    }

    ++(*index);
    *value = arguments.at(*index);
    return true;
}

std::optional<QSize> parseWindowSize(const QString& value)
{
    static const QRegularExpression pattern(QStringLiteral(R"(^([0-9]+)[xX]([0-9]+)$)"));
    const QRegularExpressionMatch match = pattern.match(value);
    if (!match.hasMatch()) {
        return std::nullopt;
    }

    bool widthOk = false;
    bool heightOk = false;
    const int width = match.captured(1).toInt(&widthOk);
    const int height = match.captured(2).toInt(&heightOk);
    if (!widthOk || !heightOk || width < 64 || height < 64) {
        return std::nullopt;
    }

    return QSize(width, height);
}

} // namespace

std::optional<RenderProbeOptions> RenderProbeOptionsParser::parse(
    const QStringList& arguments,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    RenderProbeOptions options;

    for (qsizetype index = 1; index < arguments.size(); ++index) {
        const QString argument = arguments.at(index);
        if (argument == QStringLiteral("--help") || argument == QStringLiteral("-h")) {
            options.showHelp = true;
            continue;
        }

        QString value;
        if (argument == QStringLiteral("--media")) {
            if (!takeValue(arguments, &index, &value, errorMessage)) {
                return std::nullopt;
            }
            options.mediaSource = value;
            continue;
        }

        if (argument == QStringLiteral("--label")) {
            if (!takeValue(arguments, &index, &value, errorMessage)) {
                return std::nullopt;
            }
            options.label = value;
            continue;
        }

        if (argument == QStringLiteral("--output")) {
            if (!takeValue(arguments, &index, &value, errorMessage)) {
                return std::nullopt;
            }
            options.outputPath = value;
            continue;
        }

        if (argument == QStringLiteral("--window-size")) {
            if (!takeValue(arguments, &index, &value, errorMessage)) {
                return std::nullopt;
            }

            const std::optional<QSize> size = parseWindowSize(value);
            if (!size.has_value()) {
                assignError(
                    errorMessage,
                    QStringLiteral("Invalid --window-size '%1'; expected WIDTHxHEIGHT with each dimension >= 64.")
                        .arg(value));
                return std::nullopt;
            }
            options.windowSize = *size;
            continue;
        }

        if (argument == QStringLiteral("--duration-ms")) {
            if (!takeValue(arguments, &index, &value, errorMessage)) {
                return std::nullopt;
            }

            bool durationOk = false;
            const int durationMs = value.toInt(&durationOk);
            if (!durationOk || durationMs < 1'000 || durationMs > 600'000) {
                assignError(
                    errorMessage,
                    QStringLiteral("Invalid --duration-ms '%1'; expected 1000..600000.")
                        .arg(value));
                return std::nullopt;
            }
            options.phaseDurationMs = durationMs;
            continue;
        }

        assignError(errorMessage, QStringLiteral("Unknown argument '%1'.").arg(argument));
        return std::nullopt;
    }

    if (!options.showHelp && options.mediaSource.trimmed().isEmpty()) {
        assignError(errorMessage, QStringLiteral("--media is required."));
        return std::nullopt;
    }

    return options;
}

QString RenderProbeOptionsParser::usage()
{
    return QStringLiteral(
        "Usage:\n"
        "  render_probe --media <path-or-url> [options]\n\n"
        "Options:\n"
        "  --label <text>          Baseline label, e.g. 1080p or 4k.\n"
        "  --output <path>         Also write the JSON report to this file.\n"
        "  --duration-ms <ms>      Measurement duration per phase (default 10000).\n"
        "  --window-size <WxH>     Windowed logical size (default 1280x720).\n"
        "  --help, -h              Show this help.\n\n"
        "Each run measures the same media twice: windowed first, then fullscreen.\n");
}

} // namespace player::tools::render_probe
