#include "app/bootstrap/graphics_backend_bootstrap.h"

#include <QQuickWindow>
#include <QSGRendererInterface>

namespace player::app {

void GraphicsBackendBootstrap::configure() noexcept
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
}

} // namespace player::app
