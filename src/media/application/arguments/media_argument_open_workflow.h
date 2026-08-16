#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QtGlobal>

namespace player::media::application {

class MediaOpenCoordinator;
class UrlOpenWorkflow;

enum class MediaArgumentOpenOutcome : quint8
{
    Ignored = 0,
    OpenedLocal,
    OpenedRemote,
    DeferredMultiple,
    RejectedLocal,
    RejectedRemote,
    RejectedUnsupported,
    OpenedMultiple,
    RejectedMultiple,
};

struct MediaArgumentOpenResult final
{
    MediaArgumentOpenOutcome outcome = MediaArgumentOpenOutcome::Ignored;
    QList<QUrl> orderedSources;

    [[nodiscard]] bool opened() const noexcept;
    [[nodiscard]] bool deferred() const noexcept;
};

class MediaArgumentOpenWorkflow final
{
public:
    MediaArgumentOpenWorkflow(
        MediaOpenCoordinator& mediaOpenCoordinator,
        UrlOpenWorkflow& urlOpenWorkflow);

    [[nodiscard]] MediaArgumentOpenResult openProcessArguments(
        const QStringList& processArguments,
        const QString& workingDirectory);

private:
    MediaOpenCoordinator& mediaOpenCoordinator_;
    UrlOpenWorkflow& urlOpenWorkflow_;
};

} // namespace player::media::application
