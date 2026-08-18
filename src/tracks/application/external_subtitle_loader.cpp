#include "tracks/application/external_subtitle_loader.h"

#include <QFileInfo>

namespace player::tracks::application {

ExternalSubtitleLoader::ExternalSubtitleLoader(QObject* parent)
    : QObject(parent)
{
}

bool ExternalSubtitleLoader::canLoad() const noexcept
{
    return canLoad_;
}

QString ExternalSubtitleLoader::lastRejectionReason() const
{
    return lastRejectionReason_;
}

bool ExternalSubtitleLoader::requestLoad(const QString& path)
{
    lastRejectionReason_.clear();

    if (!canLoad_) {
        lastRejectionReason_ = QStringLiteral("No active media is available for an external subtitle.");
        emit stateChanged();
        return false;
    }

    if (path.trimmed().isEmpty()) {
        lastRejectionReason_ = QStringLiteral("Subtitle path is empty.");
        emit stateChanged();
        return false;
    }

    if (!hasSupportedExtension(path)) {
        lastRejectionReason_ = QStringLiteral("Unsupported subtitle file type.");
        emit stateChanged();
        return false;
    }

    const QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
        lastRejectionReason_ = QStringLiteral("Subtitle file is missing or unreadable.");
        emit stateChanged();
        return false;
    }

    const QString canonical = canonicalPathOf(path);
    if (isDuplicate(canonical)) {
        lastRejectionReason_ = QStringLiteral("This subtitle is already loaded.");
        emit stateChanged();
        return false;
    }

    emit stateChanged();
    emit externalSubtitleLoadRequested(canonical);
    return true;
}

void ExternalSubtitleLoader::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const bool previousCanLoad = canLoad_;
    const QStringList previousFilenames = loadedExternalFilenames_;

    loadedExternalFilenames_.clear();
    for (const auto& track : snapshot.tracks().tracks) {
        if (track.kind != player::playback::domain::TrackKind::Subtitle
            || !track.external
            || !track.externalFilename.has_value()) {
            continue;
        }
        loadedExternalFilenames_.append(canonicalPathOf(*track.externalFilename));
    }

    canLoad_ = snapshot.lifecycle() == player::playback::domain::PlaybackLifecycleState::Ready
        || snapshot.lifecycle() == player::playback::domain::PlaybackLifecycleState::Ended;

    if (previousCanLoad != canLoad_ || previousFilenames != loadedExternalFilenames_) {
        emit stateChanged();
    }
}

bool ExternalSubtitleLoader::isDuplicate(const QString& canonicalPath) const
{
    for (const QString& existing : loadedExternalFilenames_) {
        if (existing == canonicalPath) {
            return true;
        }
    }
    return false;
}

bool ExternalSubtitleLoader::hasSupportedExtension(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QStringLiteral("srt")
        || suffix == QStringLiteral("ass")
        || suffix == QStringLiteral("ssa")
        || suffix == QStringLiteral("sub")
        || suffix == QStringLiteral("vtt");
}

QString ExternalSubtitleLoader::canonicalPathOf(const QString& path)
{
    return QFileInfo(path).absoluteFilePath();
}

} // namespace player::tracks::application
