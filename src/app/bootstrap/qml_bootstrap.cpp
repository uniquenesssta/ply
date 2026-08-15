#include "app/bootstrap/qml_bootstrap.h"

#include "foundation/logging/log_categories.h"

#include <QList>
#include <QLoggingCategory>
#include <QObject>
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

void QmlBootstrap::setInitialProperties(const QVariantMap& properties)
{
    engine_.setInitialProperties(properties);
}

bool QmlBootstrap::load()
{
    warningMessages_.clear();
    lastError_.clear();

    engine_.loadFromModule("Player.Presentation", "App");

    if (engine_.rootObjects().isEmpty()) {
        lastError_ = warningMessages_.isEmpty()
            ? QStringLiteral("Failed to create the Player.Presentation/App root QML object without a QQmlEngine diagnostic.")
            : warningMessages_.join(QLatin1Char('\n'));
        return false;
    }

    return true;
}

QObject* QmlBootstrap::rootObject() const noexcept
{
    return engine_.rootObjects().isEmpty() ? nullptr : engine_.rootObjects().constFirst();
}

const QString& QmlBootstrap::lastError() const noexcept
{
    return lastError_;
}

} // namespace player::app
