#pragma once

#include <QLoggingCategory>

namespace player::logging {

Q_DECLARE_LOGGING_CATEGORY(appLifecycle)
Q_DECLARE_LOGGING_CATEGORY(appBootstrap)
Q_DECLARE_LOGGING_CATEGORY(playbackMpv)
Q_DECLARE_LOGGING_CATEGORY(playbackRender)
Q_DECLARE_LOGGING_CATEGORY(uiInteraction)
Q_DECLARE_LOGGING_CATEGORY(persistence)

} // namespace player::logging
