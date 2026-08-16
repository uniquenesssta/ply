#include "media/application/drop/media_drop_handler.h"

#include "media/application/open/media_open_coordinator.h"

#include <QFileInfo>
#include <QVariant>

namespace player::media::application {
namespace {

const QString kOpened = QStringLiteral("opened");
const QString kMultipleFilesDeferred = QStringLiteral("multiple-files-deferred");
const QString kUrlDeferred = QStringLiteral("url-deferred");
const QString kDirectoryRejected = QStringLiteral("directory-rejected");
const QString kUnsupported = QStringLiteral("unsupported");
const QString kOpenRejected = QStringLiteral("open-rejected");
const QString kEmpty = QStringLiteral("empty");

} // namespace

MediaDropHandler::MediaDropHandler(
    MediaOpenCoordinator& mediaOpenCoordinator,
    QObject* parent)
    : QObject(parent)
    , mediaOpenCoordinator_(mediaOpenCoordinator)
{
}

QString MediaDropHandler::lastOutcomeKey() const
{
    return lastOutcomeKey_;
}

QVariantList MediaDropHandler::orderedSourceUrls() const
{
    return orderedSourceUrls_;
}

bool MediaDropHandler::canHandle(const QList<QUrl>& sourceUrls) const
{
    const Classification classification = classify(sourceUrls);
    return classification == Classification::SingleLocalFile
        || classification == Classification::MultipleLocalFiles
        || classification == Classification::RemoteUrl;
}

bool MediaDropHandler::handleDrop(const QList<QUrl>& sourceUrls)
{
    const Classification classification = classify(sourceUrls);

    switch (classification) {
    case Classification::SingleLocalFile:
        if (mediaOpenCoordinator_.openLocalFile(sourceUrls.front())) {
            setResult(kOpened, sourceUrls);
            return true;
        }
        setResult(kOpenRejected, sourceUrls);
        emit dropRejected(lastOutcomeKey_);
        return false;
    case Classification::MultipleLocalFiles:
        setResult(kMultipleFilesDeferred, sourceUrls);
        emit dropDeferred(lastOutcomeKey_, orderedSourceUrls_);
        return false;
    case Classification::RemoteUrl:
        setResult(kUrlDeferred, sourceUrls);
        emit dropDeferred(lastOutcomeKey_, orderedSourceUrls_);
        return false;
    case Classification::Directory:
        setResult(kDirectoryRejected, sourceUrls);
        emit dropRejected(lastOutcomeKey_);
        return false;
    case Classification::Unsupported:
        setResult(kUnsupported, sourceUrls);
        emit dropRejected(lastOutcomeKey_);
        return false;
    case Classification::Empty:
        setResult(kEmpty, sourceUrls);
        emit dropRejected(lastOutcomeKey_);
        return false;
    }

    return false;
}

MediaDropHandler::Classification MediaDropHandler::classify(
    const QList<QUrl>& sourceUrls) const
{
    if (sourceUrls.isEmpty()) {
        return Classification::Empty;
    }

    bool containsRemoteUrl = false;
    qsizetype localFileCount = 0;

    for (const QUrl& sourceUrl : sourceUrls) {
        if (!sourceUrl.isValid() || sourceUrl.isEmpty()) {
            return Classification::Unsupported;
        }

        if (sourceUrl.isLocalFile()) {
            const QFileInfo info(sourceUrl.toLocalFile());
            if (info.exists() && info.isDir()) {
                return Classification::Directory;
            }
            ++localFileCount;
            continue;
        }

        const QString scheme = sourceUrl.scheme().toLower();
        if (scheme == QStringLiteral("http") || scheme == QStringLiteral("https")) {
            containsRemoteUrl = true;
            continue;
        }

        return Classification::Unsupported;
    }

    if (containsRemoteUrl) {
        return Classification::RemoteUrl;
    }

    return localFileCount == 1
        ? Classification::SingleLocalFile
        : Classification::MultipleLocalFiles;
}

void MediaDropHandler::setResult(
    const QString& outcomeKey,
    const QList<QUrl>& sourceUrls)
{
    QVariantList orderedSourceUrls;
    orderedSourceUrls.reserve(sourceUrls.size());
    for (const QUrl& sourceUrl : sourceUrls) {
        orderedSourceUrls.push_back(QVariant::fromValue(sourceUrl));
    }

    if (lastOutcomeKey_ == outcomeKey && orderedSourceUrls_ == orderedSourceUrls) {
        return;
    }

    lastOutcomeKey_ = outcomeKey;
    orderedSourceUrls_ = std::move(orderedSourceUrls);
    emit dropResultChanged();
}

} // namespace player::media::application
