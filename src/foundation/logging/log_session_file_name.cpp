#include "foundation/logging/log_session_file_name.h"

#include <QFileInfo>

namespace player::logging {

QString makeLogSessionFileName(
    const QString& baseFileName,
    const QDateTime& localDateTime,
    int sequence)
{
    const QFileInfo fileInfo(baseFileName);
    const QString stem = fileInfo.completeBaseName().isEmpty()
        ? QStringLiteral("player")
        : fileInfo.completeBaseName();
    const QString extension = fileInfo.suffix();
    const QString timestamp = localDateTime.toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const int normalizedSequence = sequence < 1 ? 1 : sequence;

    QString fileName = QStringLiteral("%1-%2").arg(stem, timestamp);
    if (normalizedSequence > 1) {
        fileName += QStringLiteral("-%1")
                        .arg(normalizedSequence, 2, 10, QLatin1Char('0'));
    }
    if (!extension.isEmpty()) {
        fileName += QLatin1Char('.');
        fileName += extension;
    }
    return fileName;
}

} // namespace player::logging
