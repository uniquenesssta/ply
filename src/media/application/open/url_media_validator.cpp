#include "media/application/open/url_media_validator.h"

#include <QUrl>

namespace player::media::application {

UrlMediaValidationResult UrlMediaValidator::validate(const QString& sourceText)
{
    const QString trimmed = sourceText.trimmed();
    if (trimmed.isEmpty()) {
        return MediaOpenError::EmptyUrl;
    }

    QUrl sourceUrl(trimmed, QUrl::StrictMode);
    if (!sourceUrl.isValid() || sourceUrl.isRelative() || sourceUrl.host().isEmpty()) {
        return MediaOpenError::InvalidUrl;
    }

    const QString scheme = sourceUrl.scheme().toLower();
    if (scheme != QStringLiteral("http") && scheme != QStringLiteral("https")) {
        return MediaOpenError::UnsupportedUrlScheme;
    }

    sourceUrl.setScheme(scheme);
    const QString normalizedUrl = sourceUrl.toString(QUrl::FullyEncoded);
    if (normalizedUrl.isEmpty()) {
        return MediaOpenError::InvalidUrl;
    }

    return player::media::domain::MediaSource::remoteUrl(normalizedUrl);
}

} // namespace player::media::application
