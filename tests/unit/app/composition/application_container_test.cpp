#include "app/composition/application_container.h"

#include "app/bootstrap/logging_bootstrap.h"

#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

namespace player::app {

class ApplicationContainerTest final : public QObject
{
    Q_OBJECT

private slots:
    void ownsResolvedRuntimePathsByValue();
    void loggingBootstrapCreatesResolvedDevelopmentLog();
    void shutdownIsIdempotent();
};

void ApplicationContainerTest::ownsResolvedRuntimePathsByValue()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const RuntimePaths expected = RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        temporaryDirectory.path());

    ApplicationContainer container(expected);

    QCOMPARE(container.runtimePaths().mode(), expected.mode());
    QCOMPARE(container.runtimePaths().executableDirectory(), expected.executableDirectory());
    QCOMPARE(container.runtimePaths().configDirectory(), expected.configDirectory());
    QCOMPARE(container.runtimePaths().dataDirectory(), expected.dataDirectory());
    QCOMPARE(container.runtimePaths().logDirectory(), expected.logDirectory());
    QCOMPARE(container.runtimePaths().screenshotDirectory(), expected.screenshotDirectory());
}

void ApplicationContainerTest::loggingBootstrapCreatesResolvedDevelopmentLog()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString projectDirectory = QDir(temporaryDirectory.path()).filePath(
        QStringLiteral("project root with spaces"));
    const QString executableDirectory = QDir(projectDirectory).filePath(
        QStringLiteral("build/windows-msvc-debug"));
    QVERIFY(QDir().mkpath(executableDirectory));

    ApplicationContainer container(RuntimePaths::resolve(
        RuntimePaths::Mode::Installed,
        executableDirectory));

    QCOMPARE(
        QDir::cleanPath(container.runtimePaths().logDirectory()),
        QDir::cleanPath(projectDirectory));

    QString error;
    QVERIFY2(
        container.loggingBootstrap().start(container.runtimePaths(), &error),
        qPrintable(error));

    const QString logPath = QDir(projectDirectory).filePath(QStringLiteral("player.log"));
    QVERIFY2(QFileInfo(logPath).isFile(), qPrintable(logPath));

    container.shutdown();
}

void ApplicationContainerTest::shutdownIsIdempotent()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    ApplicationContainer container(RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        temporaryDirectory.path()));

    container.shutdown();
    container.shutdown();

    QVERIFY(true);
}

} // namespace player::app

QTEST_GUILESS_MAIN(player::app::ApplicationContainerTest)
#include "application_container_test.moc"
