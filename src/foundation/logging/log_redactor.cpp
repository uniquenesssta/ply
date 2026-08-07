#include "foundation/logging/log_redactor.h"

#include <QDir>
#include <QRegularExpression>

namespace player::logging {

QString LogRedactor::redact(QString text)
{
    text.replace(
        QRegularExpression(
            QStringLiteral("(?i)\\b(token|access_token|refresh_token|api[_-]?key|password|authorization)=([^\\s&]+)")),
        QStringLiteral("\\1=<redacted>"));

    text.replace(
        QRegularExpression(QStringLiteral("(?i)\\bBearer\\s+[A-Za-z0-9._~+/\\-=]+")),
        QStringLiteral("Bearer <redacted>"));

    text.replace(
        QRegularExpression(QStringLiteral("://[^/\\s:@]+:[^@\\s/]+@")),
        QStringLiteral("://<redacted>@"));

    const QString home = QDir::cleanPath(QDir::fromNativeSeparators(QDir::homePath()));
    if (!home.isEmpty() && home != QStringLiteral(".")) {
        QString normalized = QDir::fromNativeSeparators(text);
        normalized.replace(home, QStringLiteral("~"), Qt::CaseInsensitive);
        text = normalized;
    }

    return text;
}

} // namespace player::logging
