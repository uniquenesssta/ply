#include "app/bootstrap/graphics_backend/graphics_backend_bootstrap.h"
#include "app/bootstrap/graphics_backend/graphics_backend_probe.h"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest>

namespace {

class GraphicsBackendProbeTest final : public QObject {
    Q_OBJECT

private slots:
    void configuredApiIsOpenGl();
    void probeCreatesContextAndReportsRenderer();
};

void GraphicsBackendProbeTest::configuredApiIsOpenGl()
{
    QCOMPARE(QQuickWindow::graphicsApi(), QSGRendererInterface::OpenGL);
}

void GraphicsBackendProbeTest::probeCreatesContextAndReportsRenderer()
{
    player::app::GraphicsBackendInfo info;
    QString errorMessage;

    const bool success = player::app::GraphicsBackendProbe::probe(info, &errorMessage);
    const QByteArray failureText = errorMessage.toLocal8Bit();
    QVERIFY2(success, failureText.constData());

    QVERIFY(!info.renderer.isEmpty());
    QVERIFY(!info.version.isEmpty());
    QVERIFY(info.majorVersion > 0);
    QVERIFY(info.minorVersion >= 0);
}

} // namespace

int main(int argc, char* argv[])
{
    player::app::GraphicsBackendBootstrap::configure();

    QGuiApplication application(argc, argv);
    GraphicsBackendProbeTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "graphics_backend_probe_test.moc"
