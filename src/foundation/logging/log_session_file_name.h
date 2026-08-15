#pragma once

#include <QDateTime>
#include <QString>

namespace player::logging {

[[nodiscard]] QString makeLogSessionFileName(
    const QString& baseFileName,
    const QDateTime& localDateTime,
    int sequence = 1);

} // namespace player::logging
