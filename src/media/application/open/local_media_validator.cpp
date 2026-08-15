#include "media/application/open/local_media_validator.h"

#include <QFileInfo>

namespace player::media::application {

LocalMediaValidationResult LocalMediaValidator::validate(const QUrl& sourceUrl)
{
    if (!sourceUrl.isLocalFile()) {
        return MediaOpenError::NotLocalFile;
    }

    const QFileInfo fileInfo(sourceUrl.toLocalFile());
    if (!fileInfo.exists()) {
        return MediaOpenError::NotFound;
    }
    if (!fileInfo.isFile()) {
        return MediaOpenError::NotRegularFile;
    }
    if (!fileInfo.isReadable()) {
        return MediaOpenError::NotReadable;
    }

    const QString canonicalPath = fileInfo.canonicalFilePath();
    if (canonicalPath.isEmpty()) {
        return MediaOpenError::CanonicalizationFailed;
    }

    return player::media::domain::MediaSource::localFile(canonicalPath);
}

} // namespace player::media::application
