#include "app/bootstrap/application_bootstrap.h"

#include "app/bootstrap/qml_bootstrap.h"

#include <QCoreApplication>
#include <QGuiApplication>

#include <cstdlib>

namespace player::app {

int ApplicationBootstrap::run(QGuiApplication& application)
{
    configureApplicationMetadata(application);

    QmlBootstrap qmlBootstrap;
    if (!qmlBootstrap.load()) {
        return EXIT_FAILURE;
    }

    return application.exec();
}

void ApplicationBootstrap::configureApplicationMetadata(QGuiApplication& application)
{
    application.setOrganizationName(QStringLiteral("ModularPlayer"));
    application.setOrganizationDomain(QStringLiteral("local.modularplayer"));
    application.setApplicationName(QStringLiteral("Player"));
    application.setApplicationVersion(QStringLiteral("0.1.0"));
}

} // namespace player::app
