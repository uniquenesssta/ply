#include "media/application/open/url_open_workflow.h"

#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_media_validator.h"

#include <variant>

namespace player::media::application {

UrlOpenWorkflow::UrlOpenWorkflow(
    MediaOpenCoordinator& mediaOpenCoordinator,
    QObject* parent)
    : QObject(parent)
    , mediaOpenCoordinator_(mediaOpenCoordinator)
{
}

QString UrlOpenWorkflow::lastErrorKey() const
{
    return mediaOpenErrorKey(lastError_);
}

bool UrlOpenWorkflow::openUrl(const QString& sourceText)
{
    UrlMediaValidationResult validation = UrlMediaValidator::validate(sourceText);
    if (const auto* error = std::get_if<MediaOpenError>(&validation)) {
        setError(*error);
        return false;
    }

    const auto& source = std::get<player::media::domain::MediaSource>(validation);
    if (!mediaOpenCoordinator_.openSource(source)) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    setError(MediaOpenError::None);
    return true;
}

void UrlOpenWorkflow::setError(MediaOpenError error)
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
