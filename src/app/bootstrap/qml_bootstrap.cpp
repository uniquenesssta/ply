#include "app/bootstrap/qml_bootstrap.h"

#include "foundation/logging/log_categories.h"
#include "presentation/qml/types/presentation_type_registration.h"

#include <QList>
#include <QLoggingCategory>
#include <QQmlEngine>
#include <QQmlError>

namespace player::app {

QmlBootstrap::QmlBootstrap()
{
    QObject::connect(
        &engine_,
        &QQmlEngine::warnings,
        &engine_,
        [this](const QList<QQmlError>& warnings) {
            for (const QQmlError& warning : warnings) {
                const QString diagnostic = warning.toString();
                warningMessages_.append(diagnostic);
                qCWarning(player::logging::appBootstrap).noquote()
                    << "QML warning:" << diagnostic;
            }
        });
}

bool QmlBootstrap::load()
{
    warningMessages_.clear();
    lastError_.clear();

    if (!player::presentation::qml::registerPresentationQmlTypes()) {
        lastError_ = QStringLiteral("Failed to register Player.Presentation C++ QML types.");
        return false;
    }

    engine_.loadFromModule("Player.Presentation", "App");

    if (engine_.rootObjects().isEmpty()) {
        lastError_ = warningMessages_.isEmpty()
            ? QStringLiteral("Failed to create the Player.Presentation/App root QML object without a QQmlEngine diagnostic.")
            : warningMessages_.join(QLatin1Char('\n'));
        return false;
    }

    return true;
}

const QString& QmlBootstrap::lastError() const noexcept
{
    return lastError_;
}

} // namespace player::app
