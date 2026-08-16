#pragma once

#include <QString>
#include <QStringList>

class QQuickWindow;

namespace player::media::application {
class MediaArgumentOpenWorkflow;
}

namespace player::app {

class StartupMediaOpenScheduler final
{
public:
    [[nodiscard]] static bool scheduleAfterFirstFrame(
        QQuickWindow* window,
        player::media::application::MediaArgumentOpenWorkflow& workflow,
        QStringList processArguments,
        QString workingDirectory,
        QString* errorMessage = nullptr);
};

} // namespace player::app
