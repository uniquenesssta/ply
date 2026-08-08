#include "playback_probe_matrix_runner.h"
#include "playback_probe_runner.h"

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QTextStream>

using player::tools::playback_probe::PlaybackProbeMatrixRunner;
using player::tools::playback_probe::PlaybackProbeRunner;

namespace {

void printUsage()
{
    QTextStream error(stderr);
    error << "Usage:" << Qt::endl;
    error << "  playback_probe <local-media-or-url>" << Qt::endl;
    error << "  playback_probe --matrix" << Qt::endl;
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("playback_probe"));

    const QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() != 2) {
        printUsage();
        return 64;
    }

    if (arguments.at(1) == QStringLiteral("--matrix")) {
        PlaybackProbeMatrixRunner runner;
        QObject::connect(
            &runner,
            &PlaybackProbeMatrixRunner::finished,
            &app,
            [](int exitCode) { QCoreApplication::exit(exitCode); });

        QString errorMessage;
        if (!runner.start(&errorMessage)) {
            QTextStream error(stderr);
            error << "[playback_probe] MATRIX FAIL: " << errorMessage << Qt::endl;
            return 1;
        }
        return app.exec();
    }

    PlaybackProbeRunner runner;
    QObject::connect(
        &runner,
        &PlaybackProbeRunner::finished,
        &app,
        [](int exitCode) { QCoreApplication::exit(exitCode); });

    QString errorMessage;
    if (!runner.start(arguments.at(1), &errorMessage)) {
        QTextStream error(stderr);
        error << "[playback_probe] FAIL: " << errorMessage << Qt::endl;
        return 1;
    }

    return app.exec();
}
