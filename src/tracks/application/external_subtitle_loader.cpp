#include "external_subtitle_loader.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>

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

bool ExternalSubtitleLoader::loadLocalSubtitle(const QUrl& sourceUrl)
{
    setLastErrorKey({});

    if (!sourceUrl.isValid() || !sourceUrl.isLocalFile()) {
        return reject(QStringLiteral("not-local-file"));
    }

    const QString localPath = sourceUrl.toLocalFile();
    if (localPath.isEmpty()) {
        return reject(QStringLiteral("not-local-file"));
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

    if (!submitExternalSubtitle_
        || !submitExternalSubtitle_(
            player::playback::domain::AddExternalSubtitleCommand{canonicalPath})) {
        return reject(QStringLiteral("submission-failed"));
    }

    return true;
}

bool ExternalSubtitleLoader::reject(QString errorKey)
{
    setLastErrorKey(std::move(errorKey));
    return false;
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
