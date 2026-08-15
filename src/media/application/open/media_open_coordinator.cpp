#include "media/application/open/media_open_coordinator.h"

#include <QFileInfo>

#include <utility>

namespace player::media::application {

MediaOpenCoordinator::MediaOpenCoordinator(SubmitMedia submitMedia, QObject* parent)
    : QObject(parent)
    , submitMedia_(std::move(submitMedia))
{
}

QString MediaOpenCoordinator::lastErrorKey() const
{
    return mediaOpenErrorKey(lastError_);
}

bool MediaOpenCoordinator::openLocalFile(const QUrl& sourceUrl)
{
    if (sourceUrl.isEmpty()) {
        return false;
    }

    if (!sourceUrl.isLocalFile()) {
        setError(MediaOpenError::NotLocalFile);
        return false;
    }

    const QFileInfo fileInfo(sourceUrl.toLocalFile());
    if (!fileInfo.exists()) {
        setError(MediaOpenError::NotFound);
        return false;
    }
    if (!fileInfo.isFile()) {
        setError(MediaOpenError::NotRegularFile);
        return false;
    }
    if (!fileInfo.isReadable()) {
        setError(MediaOpenError::NotReadable);
        return false;
    }

    const QString canonicalPath = fileInfo.canonicalFilePath();
    if (canonicalPath.isEmpty()) {
        setError(MediaOpenError::CanonicalizationFailed);
        return false;
    }

    const player::media::domain::MediaSource source =
        player::media::domain::MediaSource::localFile(canonicalPath);
    if (!source.isValid() || !submitMedia_ || !submitMedia_(source)) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    setError(MediaOpenError::None);
    return true;
}

void MediaOpenCoordinator::setError(MediaOpenError error)
{
    if (lastError_ == error) {
        if (error != MediaOpenError::None) {
            emit openRejected(mediaOpenErrorKey(error));
        }
        return;
    }

    lastError_ = error;
    emit lastErrorKeyChanged();
    if (error != MediaOpenError::None) {
        emit openRejected(mediaOpenErrorKey(error));
    }
}

} // namespace player::media::application
