#include "app/bootstrap/startup_media_open_scheduler.h"

#include "media/application/arguments/media_argument_open_workflow.h"
#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"

#include <QCoreApplication>
#include <QFile>
#include <QMetaObject>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QtTest>

namespace player::app {

class StartupMediaOpenSchedulerTest final : public QObject
{
    Q_OBJECT

private slots:
    void noMediaArgumentsNeedNoWindow();
    void mediaArgumentsRequireWindow();
    void openIsDeferredUntilFirstFrameAndRunsOnlyOnce();
};

void StartupMediaOpenSchedulerTest::noMediaArgumentsNeedNoWindow()
{
    player::media::application::MediaOpenCoordinator coordinator(
        [](const player::media::domain::MediaSource&) { return true; });
    player::media::application::UrlOpenWorkflow urlWorkflow(coordinator);
    player::media::application::MediaArgumentOpenWorkflow workflow(coordinator, urlWorkflow);

    QString error;
    QVERIFY(StartupMediaOpenScheduler::scheduleAfterFirstFrame(
        nullptr,
        workflow,
        {QStringLiteral("Player.exe")},
        QString{},
        &error));
    QVERIFY(error.isEmpty());
}

void StartupMediaOpenSchedulerTest::mediaArgumentsRequireWindow()
{
    player::media::application::MediaOpenCoordinator coordinator(
        [](const player::media::domain::MediaSource&) { return true; });
    player::media::application::UrlOpenWorkflow urlWorkflow(coordinator);
    player::media::application::MediaArgumentOpenWorkflow workflow(coordinator, urlWorkflow);

    QString error;
    QVERIFY(!StartupMediaOpenScheduler::scheduleAfterFirstFrame(
        nullptr,
        workflow,
        {QStringLiteral("Player.exe"), QStringLiteral("sample.mp4")},
        QString{},
        &error));
    QVERIFY(!error.isEmpty());
}

void StartupMediaOpenSchedulerTest::openIsDeferredUntilFirstFrameAndRunsOnlyOnce()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("startup.mp4"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("media");
    file.close();

    int submissions = 0;
    player::media::application::MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });
    player::media::application::UrlOpenWorkflow urlWorkflow(coordinator);
    player::media::application::MediaArgumentOpenWorkflow workflow(coordinator, urlWorkflow);
    QQuickWindow window;

    QString error;
    QVERIFY(StartupMediaOpenScheduler::scheduleAfterFirstFrame(
        &window,
        workflow,
        {QStringLiteral("Player.exe"), path},
        directory.path(),
        &error));
    QVERIFY(error.isEmpty());
    QCOMPARE(submissions, 0);

    QVERIFY(QMetaObject::invokeMethod(&window, "frameSwapped", Qt::DirectConnection));
    QTRY_COMPARE_WITH_TIMEOUT(submissions, 1, 1000);

    QVERIFY(QMetaObject::invokeMethod(&window, "frameSwapped", Qt::DirectConnection));
    QCoreApplication::processEvents();
    QCOMPARE(submissions, 1);
}

} // namespace player::app

QTEST_MAIN(player::app::StartupMediaOpenSchedulerTest)
#include "startup_media_open_scheduler_test.moc"
