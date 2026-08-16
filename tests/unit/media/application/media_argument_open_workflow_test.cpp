#include "media/application/arguments/media_argument_open_workflow.h"
#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

namespace player::media::application {

class MediaArgumentOpenWorkflowTest final : public QObject
{
    Q_OBJECT

private slots:
    void singleLocalPathUsesCoordinator();
    void singleRemoteUrlUsesUrlWorkflow();
    void multipleSourcesAreDeferredWithoutRepeatedLoad();
    void unsupportedSchemeIsRejected();
    void missingLocalPathIsRejected();
};

void MediaArgumentOpenWorkflowTest::singleLocalPathUsesCoordinator()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("sample.mp4"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("media");
    file.close();

    int submissions = 0;
    player::media::domain::MediaSourceKind submittedKind =
        player::media::domain::MediaSourceKind::RemoteUrl;
    QString submittedLocation;
    MediaOpenCoordinator coordinator([&](const player::media::domain::MediaSource& source) {
        ++submissions;
        submittedKind = source.kind();
        submittedLocation = source.location();
        return true;
    });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaArgumentOpenWorkflow workflow(coordinator, urlWorkflow);

    const MediaArgumentOpenResult result = workflow.openProcessArguments(
        {QStringLiteral("Player.exe"), path},
        directory.path());

    QVERIFY(result.opened());
    QVERIFY(!result.deferred());
    QCOMPARE(
        static_cast<int>(result.outcome),
        static_cast<int>(MediaArgumentOpenOutcome::OpenedLocal));
    QCOMPARE(submissions, 1);
    QCOMPARE(
        static_cast<int>(submittedKind),
        static_cast<int>(player::media::domain::MediaSourceKind::LocalFile));
    QCOMPARE(QDir::cleanPath(submittedLocation), QDir::cleanPath(path));
}

void MediaArgumentOpenWorkflowTest::singleRemoteUrlUsesUrlWorkflow()
{
    int submissions = 0;
    player::media::domain::MediaSourceKind submittedKind =
        player::media::domain::MediaSourceKind::LocalFile;
    QString submittedLocation;
    MediaOpenCoordinator coordinator([&](const player::media::domain::MediaSource& source) {
        ++submissions;
        submittedKind = source.kind();
        submittedLocation = source.location();
        return true;
    });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaArgumentOpenWorkflow workflow(coordinator, urlWorkflow);

    const MediaArgumentOpenResult result = workflow.openProcessArguments(
        {QStringLiteral("Player.exe"), QStringLiteral("https://example.com/video.mp4")},
        QDir::currentPath());

    QVERIFY(result.opened());
    QCOMPARE(
        static_cast<int>(result.outcome),
        static_cast<int>(MediaArgumentOpenOutcome::OpenedRemote));
    QCOMPARE(submissions, 1);
    QCOMPARE(
        static_cast<int>(submittedKind),
        static_cast<int>(player::media::domain::MediaSourceKind::RemoteUrl));
    QCOMPARE(submittedLocation, QStringLiteral("https://example.com/video.mp4"));
}

void MediaArgumentOpenWorkflowTest::multipleSourcesAreDeferredWithoutRepeatedLoad()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    int submissions = 0;
    MediaOpenCoordinator coordinator([&](const player::media::domain::MediaSource&) {
        ++submissions;
        return true;
    });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaArgumentOpenWorkflow workflow(coordinator, urlWorkflow);

    const MediaArgumentOpenResult result = workflow.openProcessArguments(
        {QStringLiteral("Player.exe"),
         QStringLiteral("b.mp4"),
         QStringLiteral("https://example.com/a.mp4")},
        directory.path());

    QVERIFY(!result.opened());
    QVERIFY(result.deferred());
    QCOMPARE(submissions, 0);
    QCOMPARE(result.orderedSources.size(), 2);
    QCOMPARE(
        QDir::cleanPath(result.orderedSources.at(0).toLocalFile()),
        QDir::cleanPath(QDir(directory.path()).absoluteFilePath(QStringLiteral("b.mp4"))));
    QCOMPARE(result.orderedSources.at(1).toString(), QStringLiteral("https://example.com/a.mp4"));
}

void MediaArgumentOpenWorkflowTest::unsupportedSchemeIsRejected()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator([&](const player::media::domain::MediaSource&) {
        ++submissions;
        return true;
    });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaArgumentOpenWorkflow workflow(coordinator, urlWorkflow);

    const MediaArgumentOpenResult result = workflow.openProcessArguments(
        {QStringLiteral("Player.exe"), QStringLiteral("ftp://example.com/video.mp4")},
        QDir::currentPath());

    QVERIFY(!result.opened());
    QCOMPARE(
        static_cast<int>(result.outcome),
        static_cast<int>(MediaArgumentOpenOutcome::RejectedUnsupported));
    QCOMPARE(submissions, 0);
}

void MediaArgumentOpenWorkflowTest::missingLocalPathIsRejected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    int submissions = 0;
    MediaOpenCoordinator coordinator([&](const player::media::domain::MediaSource&) {
        ++submissions;
        return true;
    });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaArgumentOpenWorkflow workflow(coordinator, urlWorkflow);

    const MediaArgumentOpenResult result = workflow.openProcessArguments(
        {QStringLiteral("Player.exe"), QStringLiteral("missing.mp4")},
        directory.path());

    QVERIFY(!result.opened());
    QCOMPARE(
        static_cast<int>(result.outcome),
        static_cast<int>(MediaArgumentOpenOutcome::RejectedLocal));
    QCOMPARE(submissions, 0);
}

} // namespace player::media::application

QTEST_GUILESS_MAIN(player::media::application::MediaArgumentOpenWorkflowTest)
#include "media_argument_open_workflow_test.moc"
