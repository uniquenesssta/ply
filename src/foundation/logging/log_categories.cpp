#include "foundation/logging/log_categories.h"

namespace player::logging {

Q_LOGGING_CATEGORY(appLifecycle, "app.lifecycle")
Q_LOGGING_CATEGORY(appBootstrap, "app.bootstrap")
Q_LOGGING_CATEGORY(playbackMpv, "playback.mpv")
Q_LOGGING_CATEGORY(playbackRender, "playback.render")
Q_LOGGING_CATEGORY(uiInteraction, "ui.interaction")
Q_LOGGING_CATEGORY(persistence, "persistence")

} // namespace player::logging
