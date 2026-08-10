#include "render_probe_options.h"
#include "render_probe_runner.h"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QString>
#include <QTextStream>

using player::tools::render_probe::RenderProbeOptions;
using player::tools::render_probe::RenderProbeOptionsParser;
using player::tools::render_probe::RenderProbeRunner;

int main(int argc, char* argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("render_probe"));

    QString parseError;
    const auto options = RenderProbeOptionsParser::parse(
        QCoreApplication::arguments(),
        &parseError);
    if (!options.has_value()) {
        QTextStream error(stderr);
        error << "[render_probe] " << parseError << Qt::endl;
        error << RenderProbeOptionsParser::usage();
        return 64;
    }

    if (options->showHelp) {
        QTextStream output(stdout);
        output << RenderProbeOptionsParser::usage();
        return 0;
    }

    RenderProbeRunner runner(*options);
    QObject::connect(
        &runner,
        &RenderProbeRunner::finished,
        &application,
        [](int exitCode) {
            QCoreApplication::exit(exitCode);
        });

    QString startError;
    if (!runner.start(&startError)) {
        QTextStream error(stderr);
        error << "[render_probe] FAIL: " << startError << Qt::endl;
        return 1;
    }

    return application.exec();
}
