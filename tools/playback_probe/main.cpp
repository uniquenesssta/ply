#include "playback_probe_runner.h"

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QTextStream>

using player::tools::playback_probe::PlaybackProbeRunner;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("playback_probe"));

    const QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() != 2) {
        QTextStream error(stderr);
        error << "Usage: playback_probe <local-media-or-url>" << Qt::endl;
        return 64;
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
