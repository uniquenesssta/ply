#include "app/composition/application_container.h"

#include <QTemporaryDir>
#include <QtTest>

namespace player::app {

class ApplicationContainerTest final : public QObject
{
    Q_OBJECT

private slots:
    void ownsResolvedRuntimePathsByValue();
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
