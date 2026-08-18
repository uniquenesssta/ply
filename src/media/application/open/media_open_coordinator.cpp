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

MediaOpenOperationId MediaOpenCoordinator::beginReplaceOpenOperation() noexcept
{
    if (!acceptingOpenOperations_) {
        return {};
    }

    const MediaOpenOperationId operationId{nextOperationValue_++};
    if (nextOperationValue_ == 0) {
        nextOperationValue_ = 1;
    }

    currentOperation_ = operationId;
    return operationId;
}

bool MediaOpenCoordinator::isOpenOperationCurrent(
    MediaOpenOperationId operationId) const noexcept
{
    return acceptingOpenOperations_
        && operationId.isValid()
        && currentOperation_ == operationId;
}

bool MediaOpenCoordinator::cancelOpenOperation(
    MediaOpenOperationId operationId) noexcept
{
    if (!isOpenOperationCurrent(operationId)) {
        return false;
    }

    currentOperation_ = {};
    return true;
}

bool MediaOpenCoordinator::acceptingOpenOperations() const noexcept
{
    return acceptingOpenOperations_;
}

void MediaOpenCoordinator::beginShutdown() noexcept
{
    acceptingOpenOperations_ = false;
    currentOperation_ = {};
}

bool MediaOpenCoordinator::completeOpenSource(
    MediaOpenOperationId operationId,
    const player::media::domain::MediaSource& source)
{
    if (!consumeOpenOperation(operationId)) {
        return false;
    }

    return submitSource(source);
}

bool MediaOpenCoordinator::completeOpenSources(
    MediaOpenOperationId operationId,
    const QList<player::media::domain::MediaSource>& sources)
{
    if (!consumeOpenOperation(operationId)) {
        return false;
    }

    return submitSources(sources);
}

bool MediaOpenCoordinator::openSource(const player::media::domain::MediaSource& source)
{
    const MediaOpenOperationId operationId = beginReplaceOpenOperation();
    if (!operationId.isValid()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    return completeOpenSource(operationId, source);
}

bool MediaOpenCoordinator::openSources(
    const QList<player::media::domain::MediaSource>& sources)
{
    const MediaOpenOperationId operationId = beginReplaceOpenOperation();
    if (!operationId.isValid()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    return completeOpenSources(operationId, sources);
}

bool MediaOpenCoordinator::openSourceUrls(const QList<QUrl>& sourceUrls)
{
    if (sourceUrls.isEmpty()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    const MediaOpenOperationId operationId = beginReplaceOpenOperation();
    if (!operationId.isValid()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    QList<player::media::domain::MediaSource> sources;
    sources.reserve(sourceUrls.size());

    for (const QUrl& sourceUrl : sourceUrls) {
        if (sourceUrl.isLocalFile()) {
            LocalMediaValidationResult validation = LocalMediaValidator::validate(sourceUrl);
            if (const auto* error = std::get_if<MediaOpenError>(&validation)) {
                return rejectOpenOperation(operationId, *error);
            }
            sources.push_back(std::get<player::media::domain::MediaSource>(validation));
            continue;
        }

        UrlMediaValidationResult validation = UrlMediaValidator::validate(
            sourceUrl.toString(QUrl::FullyEncoded));
        if (const auto* error = std::get_if<MediaOpenError>(&validation)) {
            return rejectOpenOperation(operationId, *error);
        }
        sources.push_back(std::get<player::media::domain::MediaSource>(validation));
    }

    return completeOpenSources(operationId, sources);
}

bool MediaOpenCoordinator::openLocalFile(const QUrl& sourceUrl)
{
    if (sourceUrl.isEmpty()) {
        return false;
    }

    const MediaOpenOperationId operationId = beginReplaceOpenOperation();
    if (!operationId.isValid()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    LocalMediaValidationResult validation = LocalMediaValidator::validate(sourceUrl);
    if (const auto* error = std::get_if<MediaOpenError>(&validation)) {
        return rejectOpenOperation(operationId, *error);
    }

    return completeOpenSource(
        operationId,
        std::get<player::media::domain::MediaSource>(validation));
}

bool MediaOpenCoordinator::openLocalFiles(const QList<QUrl>& sourceUrls)
{
    if (sourceUrls.isEmpty()) {
        return false;
    }

    const MediaOpenOperationId operationId = beginReplaceOpenOperation();
    if (!operationId.isValid()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    QList<player::media::domain::MediaSource> sources;
    sources.reserve(sourceUrls.size());

    for (const QUrl& sourceUrl : sourceUrls) {
        if (!sourceUrl.isLocalFile()) {
            return rejectOpenOperation(operationId, MediaOpenError::NotLocalFile);
        }

        LocalMediaValidationResult validation = LocalMediaValidator::validate(sourceUrl);
        if (const auto* error = std::get_if<MediaOpenError>(&validation)) {
            return rejectOpenOperation(operationId, *error);
        }
        sources.push_back(std::get<player::media::domain::MediaSource>(validation));
    }

    return completeOpenSources(operationId, sources);
}

bool MediaOpenCoordinator::consumeOpenOperation(
    MediaOpenOperationId operationId) noexcept
{
    if (!isOpenOperationCurrent(operationId)) {
        return false;
    }

    currentOperation_ = {};
    return true;
}

bool MediaOpenCoordinator::rejectOpenOperation(
    MediaOpenOperationId operationId,
    MediaOpenError error)
{
    if (!consumeOpenOperation(operationId)) {
        return false;
    }

    setError(error);
    return false;
}

bool MediaOpenCoordinator::submitSource(
    const player::media::domain::MediaSource& source)
{
    if (!source.isValid() || !submitMedia_ || !submitMedia_(source)) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    setError(MediaOpenError::None);
    return true;
}

bool MediaOpenCoordinator::submitSources(
    const QList<player::media::domain::MediaSource>& sources)
{
    if (sources.isEmpty()) {
        setError(MediaOpenError::SubmissionRejected);
        return false;
    }

    if (sources.size() == 1) {
        return submitSource(sources.constFirst());
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
