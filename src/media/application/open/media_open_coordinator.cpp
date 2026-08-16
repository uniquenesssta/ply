#include "media/application/open/media_open_coordinator.h"

#include "media/application/open/local_media_validator.h"
#include "media/application/open/url_media_validator.h"

#include <utility>
#include <variant>

namespace player::media::application {

MediaOpenCoordinator::MediaOpenCoordinator(SubmitMedia submitMedia, QObject* parent)
    : MediaOpenCoordinator(std::move(submitMedia), SubmitMediaBatch{}, parent)
{
}

MediaOpenCoordinator::MediaOpenCoordinator(
    SubmitMedia submitMedia,
    SubmitMediaBatch submitMediaBatch,
    QObject* parent)
    : QObject(parent)
    , submitMedia_(std::move(submitMedia))
    , submitMediaBatch_(std::move(submitMediaBatch))
{
}

QString MediaOpenCoordinator::lastErrorKey() const
{
    return mediaOpenErrorKey(lastError_);
}

bool MediaOpenCoordinator::openSource(const player::media::domain::MediaSource& source)
{
    if (!source.isValid() || !submitMedia_ || !submitMedia_(source)) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    setError(MediaOpenError::None);
    return true;
}

bool MediaOpenCoordinator::openSources(
    const QList<player::media::domain::MediaSource>& sources)
{
    if (sources.isEmpty()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    if (sources.size() == 1) {
        return openSource(sources.constFirst());
    }

    for (const player::media::domain::MediaSource& source : sources) {
        if (!source.isValid()) {
            setError(MediaOpenError::SubmissionRejected);
            return false;
        }
    }

    if (!submitMediaBatch_ || !submitMediaBatch_(sources)) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    setError(MediaOpenError::None);
    return true;
}

bool MediaOpenCoordinator::openSourceUrls(const QList<QUrl>& sourceUrls)
{
    if (sourceUrls.isEmpty()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    QList<player::media::domain::MediaSource> sources;
    sources.reserve(sourceUrls.size());

    for (const QUrl& sourceUrl : sourceUrls) {
        if (sourceUrl.isLocalFile()) {
            LocalMediaValidationResult validation = LocalMediaValidator::validate(sourceUrl);
            if (const auto* error = std::get_if<MediaOpenError>(&validation)) {
                setError(*error);
                return false;
            }
            sources.push_back(std::get<player::media::domain::MediaSource>(validation));
            continue;
        }

        UrlMediaValidationResult validation = UrlMediaValidator::validate(
            sourceUrl.toString(QUrl::FullyEncoded));
        if (const auto* error = std::get_if<MediaOpenError>(&validation)) {
            setError(*error);
            return false;
        }
        sources.push_back(std::get<player::media::domain::MediaSource>(validation));
    }

    return openSources(sources);
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

    return openSource(std::get<player::media::domain::MediaSource>(validation));
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
