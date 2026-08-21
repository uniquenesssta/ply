#include "external_subtitle_loader.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QtGlobal>

#include <utility>

namespace player::tracks::application {
namespace {

bool isSupportedSubtitleSuffix(const QString& suffix)
{
    return suffix.compare(QStringLiteral("srt"), Qt::CaseInsensitive) == 0
        || suffix.compare(QStringLiteral("ass"), Qt::CaseInsensitive) == 0;
}

} // namespace

ExternalSubtitleLoader::ExternalSubtitleLoader(
    SubmitExternalSubtitle submitExternalSubtitle,
    QObject* parent)
    : QObject(parent)
    , submitExternalSubtitle_(std::move(submitExternalSubtitle))
{
}

QString ExternalSubtitleLoader::lastErrorKey() const
{
    return lastErrorKey_;
}

bool ExternalSubtitleLoader::loading() const noexcept
{
    return !pendingLoads_.isEmpty();
}

bool ExternalSubtitleLoader::loadLocalSubtitle(const QUrl& sourceUrl)
{
    setLastErrorKey({});

    if (shuttingDown_) {
        return reject(QStringLiteral("shutting-down"));
    }

    if (!generation_.isValid()) {
        return reject(QStringLiteral("no-active-media"));
    }

    if (!sourceUrl.isValid() || !sourceUrl.isLocalFile()) {
        return reject(QStringLiteral("not-local-file"));
    }

    const QString localPath = sourceUrl.toLocalFile();
    if (localPath.isEmpty()) {
        return reject(QStringLiteral("not-local-file"));
    }

    const QString requestedSourceKey = localPathKey(localPath);
    if (!requestedSourceKey.isEmpty() && containsSourceKey(requestedSourceKey)) {
        qInfo().noquote()
            << "External subtitle load deduplicated for current media generation.";
        return true;
    }

    const QFileInfo fileInfo(localPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return reject(QStringLiteral("file-unavailable"));
    }
    if (!isSupportedSubtitleSuffix(fileInfo.suffix())) {
        return reject(QStringLiteral("unsupported-format"));
    }

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return reject(QStringLiteral("file-unavailable"));
    }
    file.close();

    const QString canonicalPath = fileInfo.canonicalFilePath();
    if (canonicalPath.isEmpty()) {
        return reject(QStringLiteral("file-unavailable"));
    }

    const QString sourceKey = localPathKey(canonicalPath);
    if (sourceKey.isEmpty()) {
        return reject(QStringLiteral("file-unavailable"));
    }

    if (containsSourceKey(sourceKey)) {
        qInfo().noquote()
            << "External subtitle load deduplicated for current media generation.";
        return true;
    }

    if (!submitExternalSubtitle_) {
        return reject(QStringLiteral("submission-failed"));
    }

    const auto requestId = submitExternalSubtitle_(
        player::playback::domain::AddExternalSubtitleCommand{canonicalPath});
    if (!requestId.has_value() || !requestId->isValid()
        || pendingLoads_.contains(requestId->value())) {
        return reject(QStringLiteral("submission-failed"));
    }

    const bool wasLoading = loading();
    pendingLoads_.insert(
        requestId->value(),
        PendingLoad{sourceKey, generation_});
    if (wasLoading != loading()) {
        emit loadingChanged();
    }

    qInfo().noquote() << "External subtitle load submitted to playback backend.";
    return true;
}

void ExternalSubtitleLoader::acceptRequestResult(
    player::ids::RequestId requestId,
    player::playback::domain::MediaGeneration generation,
    bool succeeded,
    const QString& diagnostic)
{
    const auto pendingIt = pendingLoads_.find(requestId.value());
    if (pendingIt == pendingLoads_.end()) {
        return;
    }

    const bool wasLoading = loading();
    const PendingLoad pending = *pendingIt;
    pendingLoads_.erase(pendingIt);
    if (wasLoading != loading()) {
        emit loadingChanged();
    }

    if (shuttingDown_
        || generation != generation_
        || pending.generation != generation_) {
        return;
    }

    if (succeeded) {
        acceptedSourceKeys_.insert(pending.sourceKey);
        setLastErrorKey({});
        return;
    }

    setLastErrorKey(QStringLiteral("backend-rejected"));
    qWarning().noquote()
        << "External subtitle playback request rejected:"
        << (diagnostic.isEmpty()
                ? QStringLiteral("No backend diagnostic was provided.")
                : diagnostic);
}

void ExternalSubtitleLoader::beginShutdown() noexcept
{
    if (shuttingDown_) {
        return;
    }

    shuttingDown_ = true;
    generation_ = {};
    acceptedSourceKeys_.clear();
    snapshotSourceKeys_.clear();
    clearPendingLoads();
    setLastErrorKey({});
}

void ExternalSubtitleLoader::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    if (shuttingDown_) {
        return;
    }

    if (snapshot.generation() != generation_) {
        generation_ = snapshot.generation();
        acceptedSourceKeys_.clear();
        snapshotSourceKeys_.clear();
        clearPendingLoads();
        setLastErrorKey({});
    }

    QSet<QString> nextSnapshotSourceKeys;
    if (generation_.isValid()) {
        for (const player::playback::domain::TrackDescriptor& track
             : snapshot.tracks().tracks) {
            if (track.kind != player::playback::domain::TrackKind::Subtitle
                || !track.external
                || !track.externalFilename.has_value()) {
                continue;
            }

            const QString sourceKey = localPathKey(*track.externalFilename);
            if (!sourceKey.isEmpty()) {
                nextSnapshotSourceKeys.insert(sourceKey);
            }
        }
    }
    snapshotSourceKeys_ = std::move(nextSnapshotSourceKeys);

    const bool wasLoading = loading();
    for (auto it = pendingLoads_.begin(); it != pendingLoads_.end();) {
        if (it->generation == generation_
            && snapshotSourceKeys_.contains(it->sourceKey)) {
            acceptedSourceKeys_.insert(it->sourceKey);
            it = pendingLoads_.erase(it);
            continue;
        }
        ++it;
    }
    if (wasLoading != loading()) {
        emit loadingChanged();
    }
}

QString ExternalSubtitleLoader::localPathKey(const QString& path)
{
    if (path.isEmpty()) {
        return {};
    }

    QString localPath = path;
    const QUrl url(path);
    if (url.isLocalFile()) {
        localPath = url.toLocalFile();
    }

    const QFileInfo fileInfo(localPath);
    QString normalized = fileInfo.canonicalFilePath();
    if (normalized.isEmpty()) {
        normalized = fileInfo.absoluteFilePath();
    }
    normalized = QDir::cleanPath(normalized);

#ifdef Q_OS_WIN
    return normalized.toCaseFolded();
#else
    return normalized;
#endif
}

bool ExternalSubtitleLoader::containsSourceKey(const QString& sourceKey) const
{
    if (acceptedSourceKeys_.contains(sourceKey)
        || snapshotSourceKeys_.contains(sourceKey)) {
        return true;
    }

    for (const PendingLoad& pending : pendingLoads_) {
        if (pending.generation == generation_
            && pending.sourceKey == sourceKey) {
            return true;
        }
    }
    return false;
}

bool ExternalSubtitleLoader::reject(QString errorKey)
{
    setLastErrorKey(std::move(errorKey));
    qWarning().noquote() << "External subtitle load rejected:" << lastErrorKey_;
    return false;
}

void ExternalSubtitleLoader::clearPendingLoads() noexcept
{
    const bool wasLoading = loading();
    pendingLoads_.clear();
    if (wasLoading != loading()) {
        emit loadingChanged();
    }
}

void ExternalSubtitleLoader::setLastErrorKey(QString errorKey)
{
    if (lastErrorKey_ == errorKey) {
        return;
    }

    lastErrorKey_ = std::move(errorKey);
    emit lastErrorKeyChanged();
}

} // namespace player::tracks::application
