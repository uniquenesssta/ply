#include "media/application/open/media_open_coordinator.h"

#include "media/application/open/local_media_validator.h"

#include <utility>
#include <variant>

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

    LocalMediaValidationResult validation = LocalMediaValidator::validate(sourceUrl);
    if (const auto* error = std::get_if<MediaOpenError>(&validation)) {
        setError(*error);
        return false;
    }

    const auto& source = std::get<player::media::domain::MediaSource>(validation);
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
