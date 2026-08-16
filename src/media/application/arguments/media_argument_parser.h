#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace player::media::application {

struct ParsedMediaArguments final
{
    QList<QUrl> orderedSources;
};

class MediaArgumentParser final
{
public:
    [[nodiscard]] static ParsedMediaArguments parseProcessArguments(
        const QStringList& processArguments,
        const QString& workingDirectory);
};

} // namespace player::media::application
