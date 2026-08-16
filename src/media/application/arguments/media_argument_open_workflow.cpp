#include "media/application/arguments/media_argument_open_workflow.h"

#include "media/application/arguments/media_argument_parser.h"
#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"

namespace player::media::application {

bool MediaArgumentOpenResult::opened() const noexcept
{
    return outcome == MediaArgumentOpenOutcome::OpenedLocal
        || outcome == MediaArgumentOpenOutcome::OpenedRemote;
}

bool MediaArgumentOpenResult::deferred() const noexcept
{
    return outcome == MediaArgumentOpenOutcome::DeferredMultiple;
}

MediaArgumentOpenWorkflow::MediaArgumentOpenWorkflow(
    MediaOpenCoordinator& mediaOpenCoordinator,
    UrlOpenWorkflow& urlOpenWorkflow)
    : mediaOpenCoordinator_(mediaOpenCoordinator)
    , urlOpenWorkflow_(urlOpenWorkflow)
{
}

MediaArgumentOpenResult MediaArgumentOpenWorkflow::openProcessArguments(
    const QStringList& processArguments,
    const QString& workingDirectory)
{
    MediaArgumentOpenResult result;
    ParsedMediaArguments parsed = MediaArgumentParser::parseProcessArguments(
        processArguments,
        workingDirectory);
    result.orderedSources = std::move(parsed.orderedSources);

    if (result.orderedSources.isEmpty()) {
        return result;
    }

    if (result.orderedSources.size() > 1) {
        result.outcome = MediaArgumentOpenOutcome::DeferredMultiple;
        return result;
    }

    const QUrl& sourceUrl = result.orderedSources.constFirst();
    if (sourceUrl.isLocalFile()) {
        result.outcome = mediaOpenCoordinator_.openLocalFile(sourceUrl)
            ? MediaArgumentOpenOutcome::OpenedLocal
            : MediaArgumentOpenOutcome::RejectedLocal;
        return result;
    }

    const QString scheme = sourceUrl.scheme().toLower();
    if (scheme == QStringLiteral("http") || scheme == QStringLiteral("https")) {
        result.outcome = urlOpenWorkflow_.openUrl(sourceUrl.toString(QUrl::FullyEncoded))
            ? MediaArgumentOpenOutcome::OpenedRemote
            : MediaArgumentOpenOutcome::RejectedRemote;
        return result;
    }

    result.outcome = MediaArgumentOpenOutcome::RejectedUnsupported;
    return result;
}

} // namespace player::media::application
