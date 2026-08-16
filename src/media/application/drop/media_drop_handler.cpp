#include "media/application/drop/media_drop_handler.h"

#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"

#include <QFileInfo>
#include <QVariant>

#include <utility>

namespace player::media::application {
namespace {

const QString kOpened = QStringLiteral("opened");
const QString kMultipleFilesDeferred = QStringLiteral("multiple-files-deferred");
const QString kMultipleSourcesDeferred = QStringLiteral("multiple-sources-deferred");
const QString kDirectoryRejected = QStringLiteral("directory-rejected");
const QString kUnsupported = QStringLiteral("unsupported");
const QString kOpenRejected = QStringLiteral("open-rejected");
const QString kEmpty = QStringLiteral("empty");

} // namespace

MediaDropHandler::MediaDropHandler(
    MediaOpenCoordinator& mediaOpenCoordinator,
    UrlOpenWorkflow& urlOpenWorkflow,
    QObject* parent)
    : QObject(parent)
    , mediaOpenCoordinator_(mediaOpenCoordinator)
    , urlOpenWorkflow_(urlOpenWorkflow)
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
        || classification == Classification::SingleRemoteUrl
        || classification == Classification::MultipleSources;
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
    case Classification::SingleRemoteUrl:
        if (urlOpenWorkflow_.openUrl(sourceUrls.front().toString(QUrl::FullyEncoded))) {
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
    case Classification::MultipleSources:
        setResult(kMultipleSourcesDeferred, sourceUrls);
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

    qsizetype localFileCount = 0;
    qsizetype remoteUrlCount = 0;

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
            ++remoteUrlCount;
            continue;
        }

        return Classification::Unsupported;
    }

    if (sourceUrls.size() > 1) {
        return remoteUrlCount == 0
            ? Classification::MultipleLocalFiles
            : Classification::MultipleSources;
    }

    if (remoteUrlCount == 1) {
        return Classification::SingleRemoteUrl;
    }

    return localFileCount == 1
        ? Classification::SingleLocalFile
        : Classification::Unsupported;
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
