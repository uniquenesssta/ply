#include "media/application/open/media_open_error.h"

#include <QString>

namespace player::media::application {

QString mediaOpenErrorKey(MediaOpenError error)
{
    switch (error) {
    case MediaOpenError::None:
        return {};
    case MediaOpenError::NotLocalFile:
        return QStringLiteral("not-local-file");
    case MediaOpenError::NotFound:
        return QStringLiteral("not-found");
    case MediaOpenError::NotRegularFile:
        return QStringLiteral("not-regular-file");
    case MediaOpenError::NotReadable:
        return QStringLiteral("not-readable");
    case MediaOpenError::CanonicalizationFailed:
        return QStringLiteral("canonicalization-failed");
    case MediaOpenError::EmptyUrl:
        return QStringLiteral("empty-url");
    case MediaOpenError::InvalidUrl:
        return QStringLiteral("invalid-url");
    case MediaOpenError::UnsupportedUrlScheme:
        return QStringLiteral("unsupported-url-scheme");
    case MediaOpenError::SubmissionRejected:
        return QStringLiteral("submission-rejected");
    }

    return QStringLiteral("unknown");
}

} // namespace player::media::application
