#include "app/bootstrap/startup_media_open_scheduler.h"

#include "media/application/arguments/media_argument_open_workflow.h"

#include <QMetaObject>
#include <QObject>
#include <QQuickWindow>

#include <utility>

namespace player::app {

bool StartupMediaOpenScheduler::scheduleAfterFirstFrame(
    QQuickWindow* window,
    player::media::application::MediaArgumentOpenWorkflow& workflow,
    QStringList processArguments,
    QString workingDirectory,
    QString* errorMessage)
{
    if (processArguments.size() <= 1) {
        return true;
    }

    if (window == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "Startup media arguments require a QQuickWindow root before they can be opened.");
        }
        return false;
    }

    const QMetaObject::Connection connection = QObject::connect(
        window,
        &QQuickWindow::frameSwapped,
        window,
        [workflowPtr = &workflow,
         arguments = std::move(processArguments),
         directory = std::move(workingDirectory)]() {
            (void)workflowPtr->openProcessArguments(arguments, directory);
        },
        static_cast<Qt::ConnectionType>(
            Qt::QueuedConnection | Qt::SingleShotConnection));

    if (!connection) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "Failed to schedule startup media arguments after the first rendered frame.");
        }
        return false;
    }

    return true;
}

} // namespace player::app
