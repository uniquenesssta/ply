#include "app/bootstrap/application_bootstrap.h"
#include "app/bootstrap/graphics_backend/graphics_backend_bootstrap.h"

#include <QGuiApplication>

int main(int argc, char* argv[])
{
    player::app::GraphicsBackendBootstrap::configure();

    QGuiApplication application(argc, argv);
    player::app::ApplicationBootstrap bootstrap;

    return bootstrap.run(application);
}
