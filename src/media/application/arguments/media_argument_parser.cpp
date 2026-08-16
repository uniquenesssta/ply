#include "media/application/arguments/media_argument_parser.h"

#include <QDir>
#include <QRegularExpression>

namespace player::media::application {
namespace {

bool hasExplicitUrlScheme(const QString& sourceText)
{
    static const QRegularExpression schemePattern(
        QStringLiteral(R"(^[A-Za-z][A-Za-z0-9+.-]*://)"));
    return schemePattern.match(sourceText).hasMatch();
}

QUrl sourceUrlForArgument(const QString& argument, const QString& workingDirectory)
{
    const QString sourceText = argument.trimmed();
    if (hasExplicitUrlScheme(sourceText)) {
        return QUrl(sourceText, QUrl::StrictMode);
    }

    const QDir baseDirectory(
        workingDirectory.isEmpty() ? QDir::currentPath() : workingDirectory);
    const QString absolutePath = QDir::isAbsolutePath(argument)
        ? QDir::cleanPath(argument)
        : QDir::cleanPath(baseDirectory.absoluteFilePath(argument));
    return QUrl::fromLocalFile(absolutePath);
}

} // namespace

ParsedMediaArguments MediaArgumentParser::parseProcessArguments(
    const QStringList& processArguments,
    const QString& workingDirectory)
{
    ParsedMediaArguments parsed;
    if (processArguments.size() <= 1) {
        return parsed;
    }

    bool positionalOnly = false;
    for (qsizetype index = 1; index < processArguments.size(); ++index) {
        const QString& argument = processArguments.at(index);
        if (argument.isEmpty()) {
            continue;
        }

        if (!positionalOnly && argument == QStringLiteral("--")) {
            positionalOnly = true;
            continue;
        }

        if (!positionalOnly && argument.startsWith(QLatin1Char('-'))) {
            continue;
        }

        parsed.orderedSources.push_back(sourceUrlForArgument(argument, workingDirectory));
    }

    return parsed;
}

} // namespace player::media::application
