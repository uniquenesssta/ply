#include "media/application/open/url_open_workflow.h"

#include "media/application/open/media_open_coordinator.h"

#include <QSignalSpy>
#include <QtTest>

namespace player::media::application {

class UrlOpenWorkflowTest final : public QObject
{
    Q_OBJECT

private slots:
    void validHttpUrlReachesCoordinatorAsRemoteSource();
    void validationFailureDoesNotSubmit();
    void submissionFailureIsReported();
    void successfulRequestClearsPreviousError();
};

void UrlOpenWorkflowTest::validHttpUrlReachesCoordinatorAsRemoteSource()
{
    int submissions = 0;
    player::media::domain::MediaSourceKind submittedKind =
        player::media::domain::MediaSourceKind::LocalFile;
    QString submittedLocation;
    MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource& source) {
            ++submissions;
            submittedKind = source.kind();
            submittedLocation = source.location();
            return true;
        });
    UrlOpenWorkflow workflow(coordinator);

    QVERIFY(workflow.openUrl(QStringLiteral("HTTPS://example.com/media/video.mp4")));
    QCOMPARE(submissions, 1);
    QCOMPARE(
        static_cast<int>(submittedKind),
        static_cast<int>(player::media::domain::MediaSourceKind::RemoteUrl));
    QCOMPARE(submittedLocation, QStringLiteral("https://example.com/media/video.mp4"));
    QCOMPARE(workflow.lastErrorKey(), QString{});
    QCOMPARE(coordinator.lastErrorKey(), QString{});
}

void UrlOpenWorkflowTest::validationFailureDoesNotSubmit()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });
    UrlOpenWorkflow workflow(coordinator);
    QSignalSpy rejectedSpy(&workflow, &UrlOpenWorkflow::openRejected);

    QVERIFY(!workflow.openUrl(QStringLiteral("ftp://example.com/media.mp4")));
    QCOMPARE(submissions, 0);
    QCOMPARE(workflow.lastErrorKey(), QStringLiteral("unsupported-url-scheme"));
    QCOMPARE(rejectedSpy.count(), 1);
}

void UrlOpenWorkflowTest::submissionFailureIsReported()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource&) {
            ++submissions;
            return false;
        });
    UrlOpenWorkflow workflow(coordinator);
    QSignalSpy rejectedSpy(&workflow, &UrlOpenWorkflow::openRejected);

    QVERIFY(!workflow.openUrl(QStringLiteral("https://example.com/media.mp4")));
    QCOMPARE(submissions, 1);
    QCOMPARE(workflow.lastErrorKey(), QStringLiteral("submission-rejected"));
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("submission-rejected"));
    QCOMPARE(rejectedSpy.count(), 1);
}

void UrlOpenWorkflowTest::successfulRequestClearsPreviousError()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });
    UrlOpenWorkflow workflow(coordinator);

    QVERIFY(!workflow.openUrl(QStringLiteral("relative/media.mp4")));
    QCOMPARE(workflow.lastErrorKey(), QStringLiteral("invalid-url"));

    QVERIFY(workflow.openUrl(QStringLiteral("https://example.com/media.mp4")));
    QCOMPARE(submissions, 1);
    QCOMPARE(workflow.lastErrorKey(), QString{});
}

} // namespace player::media::application

QTEST_GUILESS_MAIN(player::media::application::UrlOpenWorkflowTest)
#include "url_open_workflow_test.moc"
