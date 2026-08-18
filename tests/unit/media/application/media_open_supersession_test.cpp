#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"

#include <QList>
#include <QSignalSpy>
#include <QString>
#include <QtTest>

namespace player::media::application {

class MediaOpenSupersessionTest final : public QObject
{
    Q_OBJECT

private slots:
    void newerOperationSupersedesOlderCompletion();
    void staleBatchCompletionDoesNotSubmit();
    void staleCompletionDoesNotOverwriteErrorState();
    void operationCompletionIsSingleUse();
    void synchronousOpenSupersedesPendingOperation();
    void urlWorkflowSupersedesPendingOperationBeforeValidation();
    void cancellationIsScopedToCurrentOperation();
    void shutdownCancelsPendingAndRejectsNewOperations();
};

void MediaOpenSupersessionTest::newerOperationSupersedesOlderCompletion()
{
    QList<QString> submittedLocations;
    MediaOpenCoordinator coordinator(
        [&submittedLocations](const player::media::domain::MediaSource& source) {
            submittedLocations.push_back(source.location());
            return true;
        });

    const MediaOpenOperationId operationA = coordinator.beginReplaceOpenOperation();
    const MediaOpenOperationId operationB = coordinator.beginReplaceOpenOperation();

    QVERIFY(operationA.isValid());
    QVERIFY(operationB.isValid());
    QVERIFY(operationA != operationB);
    QVERIFY(!coordinator.isOpenOperationCurrent(operationA));
    QVERIFY(coordinator.isOpenOperationCurrent(operationB));

    const auto sourceA = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/a.mp4"));
    const auto sourceB = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/b.mp4"));

    QVERIFY(!coordinator.completeOpenSource(operationA, sourceA));
    QCOMPARE(submittedLocations.size(), 0);

    QVERIFY(coordinator.completeOpenSource(operationB, sourceB));
    QCOMPARE(submittedLocations, QList<QString>{QStringLiteral("https://example.com/b.mp4")});
    QVERIFY(!coordinator.isOpenOperationCurrent(operationB));
}

void MediaOpenSupersessionTest::staleBatchCompletionDoesNotSubmit()
{
    int singleSubmissions = 0;
    int batchSubmissions = 0;
    MediaOpenCoordinator coordinator(
        [&singleSubmissions](const player::media::domain::MediaSource&) {
            ++singleSubmissions;
            return true;
        },
        [&batchSubmissions](const QList<player::media::domain::MediaSource>&) {
            ++batchSubmissions;
            return true;
        });

    const MediaOpenOperationId operationA = coordinator.beginReplaceOpenOperation();
    const MediaOpenOperationId operationB = coordinator.beginReplaceOpenOperation();
    const QList<player::media::domain::MediaSource> sourcesA{
        player::media::domain::MediaSource::remoteUrl(
            QStringLiteral("https://example.com/a1.mp4")),
        player::media::domain::MediaSource::remoteUrl(
            QStringLiteral("https://example.com/a2.mp4")),
    };
    const QList<player::media::domain::MediaSource> sourcesB{
        player::media::domain::MediaSource::remoteUrl(
            QStringLiteral("https://example.com/b1.mp4")),
        player::media::domain::MediaSource::remoteUrl(
            QStringLiteral("https://example.com/b2.mp4")),
    };

    QVERIFY(!coordinator.completeOpenSources(operationA, sourcesA));
    QCOMPARE(singleSubmissions, 0);
    QCOMPARE(batchSubmissions, 0);

    QVERIFY(coordinator.completeOpenSources(operationB, sourcesB));
    QCOMPARE(singleSubmissions, 0);
    QCOMPARE(batchSubmissions, 1);
}

void MediaOpenSupersessionTest::staleCompletionDoesNotOverwriteErrorState()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });
    QSignalSpy rejectedSpy(&coordinator, &MediaOpenCoordinator::openRejected);

    QVERIFY(!coordinator.openSource(player::media::domain::MediaSource::remoteUrl({})));
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("submission-rejected"));
    QCOMPARE(rejectedSpy.count(), 1);

    const MediaOpenOperationId operationA = coordinator.beginReplaceOpenOperation();
    const MediaOpenOperationId operationB = coordinator.beginReplaceOpenOperation();
    const auto sourceA = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/a.mp4"));
    const auto sourceB = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/b.mp4"));

    QVERIFY(!coordinator.completeOpenSource(operationA, sourceA));
    QCOMPARE(submissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("submission-rejected"));
    QCOMPARE(rejectedSpy.count(), 1);

    QVERIFY(coordinator.completeOpenSource(operationB, sourceB));
    QCOMPARE(submissions, 1);
    QCOMPARE(coordinator.lastErrorKey(), QString{});
}

void MediaOpenSupersessionTest::operationCompletionIsSingleUse()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });

    const MediaOpenOperationId operation = coordinator.beginReplaceOpenOperation();
    const auto source = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/video.mp4"));

    QVERIFY(coordinator.completeOpenSource(operation, source));
    QVERIFY(!coordinator.completeOpenSource(operation, source));
    QCOMPARE(submissions, 1);
}

void MediaOpenSupersessionTest::synchronousOpenSupersedesPendingOperation()
{
    QList<QString> submittedLocations;
    MediaOpenCoordinator coordinator(
        [&submittedLocations](const player::media::domain::MediaSource& source) {
            submittedLocations.push_back(source.location());
            return true;
        });

    const MediaOpenOperationId pending = coordinator.beginReplaceOpenOperation();
    const auto sourceA = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/a.mp4"));
    const auto sourceB = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/b.mp4"));

    QVERIFY(coordinator.openSource(sourceB));
    QVERIFY(!coordinator.completeOpenSource(pending, sourceA));
    QCOMPARE(submittedLocations, QList<QString>{QStringLiteral("https://example.com/b.mp4")});
}

void MediaOpenSupersessionTest::urlWorkflowSupersedesPendingOperationBeforeValidation()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });
    UrlOpenWorkflow workflow(coordinator);

    const MediaOpenOperationId pending = coordinator.beginReplaceOpenOperation();
    const auto sourceA = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/a.mp4"));

    QVERIFY(!workflow.openUrl(QStringLiteral("not-a-supported-url")));
    QVERIFY(!coordinator.completeOpenSource(pending, sourceA));
    QCOMPARE(submissions, 0);
}

void MediaOpenSupersessionTest::cancellationIsScopedToCurrentOperation()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });

    const MediaOpenOperationId operationA = coordinator.beginReplaceOpenOperation();
    const MediaOpenOperationId operationB = coordinator.beginReplaceOpenOperation();

    QVERIFY(!coordinator.cancelOpenOperation(operationA));
    QVERIFY(coordinator.isOpenOperationCurrent(operationB));
    QVERIFY(coordinator.cancelOpenOperation(operationB));
    QVERIFY(!coordinator.isOpenOperationCurrent(operationB));

    const auto source = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/video.mp4"));
    QVERIFY(!coordinator.completeOpenSource(operationB, source));
    QCOMPARE(submissions, 0);
}

void MediaOpenSupersessionTest::shutdownCancelsPendingAndRejectsNewOperations()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });

    const MediaOpenOperationId pending = coordinator.beginReplaceOpenOperation();
    QVERIFY(coordinator.acceptingOpenOperations());
    QVERIFY(coordinator.isOpenOperationCurrent(pending));

    coordinator.beginShutdown();

    QVERIFY(!coordinator.acceptingOpenOperations());
    QVERIFY(!coordinator.isOpenOperationCurrent(pending));
    QVERIFY(!coordinator.beginReplaceOpenOperation().isValid());

    const auto source = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/video.mp4"));
    QVERIFY(!coordinator.completeOpenSource(pending, source));
    QVERIFY(!coordinator.openSource(source));
    QCOMPARE(submissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("submission-rejected"));
}

} // namespace player::media::application

QTEST_GUILESS_MAIN(player::media::application::MediaOpenSupersessionTest)
#include "media_open_supersession_test.moc"
