#include "app/bootstrap/qml_bootstrap.h"

#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(qmlBootstrapLog, "player.app.qml")
}

namespace player::app {

bool QmlBootstrap::load()
{
    engine_.loadFromModule("Player.Presentation", "App");

    if (engine_.rootObjects().isEmpty()) {
        qCCritical(qmlBootstrapLog) << "Failed to create the root QML object";
        return false;
    }

    return true;
}

} // namespace player::app
