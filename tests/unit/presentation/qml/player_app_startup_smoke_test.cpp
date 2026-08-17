#include <QCoreApplication>
#include <QProcess>
#include <QString>
#include <QTextStream>

namespace {

void writeProcessDiagnostics(QProcess& process)
{
    QTextStream errorStream(stderr);
    const QByteArray standardError = process.readAllStandardError();
    const QByteArray standardOutput = process.readAllStandardOutput();

    errorStream << "Player exited before the startup smoke window elapsed.\n"
                << "exitStatus=" << static_cast<int>(process.exitStatus())
                << " exitCode=" << process.exitCode() << '\n';

    if (!standardError.isEmpty()) {
        errorStream << "stderr:\n"
                    << QString::fromLocal8Bit(standardError) << '\n';
    }
    if (!standardOutput.isEmpty()) {
        errorStream << "stdout:\n"
                    << QString::fromLocal8Bit(standardOutput) << '\n';
    }
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    (void)application;

    QTextStream errorStream(stderr);

    if (argc != 2) {
        errorStream << "Expected Player executable path as the only argument.\n";
        return 2;
    }

    QProcess playerProcess;
    playerProcess.setProgram(QString::fromLocal8Bit(argv[1]));
    playerProcess.setProcessChannelMode(QProcess::SeparateChannels);
    playerProcess.start();

    if (!playerProcess.waitForStarted(5000)) {
        errorStream << "Player process did not start: "
                    << playerProcess.errorString() << '\n';
        return 1;
    }

    if (playerProcess.waitForFinished(2500)) {
        writeProcessDiagnostics(playerProcess);
        return 1;
    }

    playerProcess.terminate();
    if (!playerProcess.waitForFinished(3000)) {
        playerProcess.kill();
        playerProcess.waitForFinished(3000);
    }

    return 0;
}
