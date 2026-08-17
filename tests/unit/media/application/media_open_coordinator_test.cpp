#include "media/application/open/media_open_coordinator.h"

#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

namespace player::media::application {

class MediaOpenCoordinatorTest final : public QObject
{
    Q_OBJECT

private slots:
    void emptySelectionIsNoOp();
    void rejectsNonLocalUrl();
    void rejectsMissingFile();
    void rejectsDirectory();
    void canonicalizesAndSubmitsExistingFile();
    void localFileBatchUsesBatchSubmissionInOrder();
    void localFileBatchRejectsNonLocalUrl();
    void validatedRemoteSourceUsesSameSubmissionBoundary();
    void invalidSourceIsRejectedBeforeSubmission();
    void submissionRejectionIsReported();
    void batchUrlsAreValidatedInOrderAndSubmittedOnce();
    void invalidBatchUrlRejectsWholeBatchBeforeSubmission();
};

void MediaOpenCoordinatorTest::emptySelectionIsNoOp()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });
    QSignalSpy rejectedSpy(&coordinator, &MediaOpenCoordinator::openRejected);

    QVERIFY(!coordinator.openLocalFile(QUrl{}));
    QVERIFY(!coordinator.openLocalFiles({}));
    QCOMPARE(submissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QString{});
    QCOMPARE(rejectedSpy.count(), 0);
}

void MediaOpenCoordinatorTest::rejectsNonLocalUrl()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });

    QVERIFY(!coordinator.openLocalFile(QUrl(QStringLiteral("https://example.invalid/video.mp4"))));
    QCOMPARE(submissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("not-local-file"));
}

void MediaOpenCoordinatorTest::rejectsMissingFile()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });

    const QString missingPath = directory.filePath(QStringLiteral("missing.mp4"));
    QVERIFY(!coordinator.openLocalFile(QUrl::fromLocalFile(missingPath)));
    QCOMPARE(submissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("not-found"));
}

void MediaOpenCoordinatorTest::rejectsDirectory()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });

    QVERIFY(!coordinator.openLocalFile(QUrl::fromLocalFile(directory.path())));
    QCOMPARE(submissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("not-regular-file"));
}

void MediaOpenCoordinatorTest::canonicalizesAndSubmitsExistingFile()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString inputPath = directory.filePath(
        QStringLiteral("fixture \u5A92\u4F53 media.mp4"));
    QFile file(inputPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("fixture"), qint64{7});
    file.close();

    const QString expectedCanonicalPath = QFileInfo(inputPath).canonicalFilePath();
    QVERIFY(!expectedCanonicalPath.isEmpty());

    int submissions = 0;
    QString submittedLocation;
    player::media::domain::MediaSourceKind submittedKind =
        player::media::domain::MediaSourceKind::LocalFile;
    MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource& source) {
            ++submissions;
            submittedKind = source.kind();
            submittedLocation = source.location();
            return true;
        });

    QVERIFY(coordinator.openLocalFile(QUrl::fromLocalFile(inputPath)));
    QCOMPARE(submissions, 1);
    QCOMPARE(
        static_cast<int>(submittedKind),
        static_cast<int>(player::media::domain::MediaSourceKind::LocalFile));
    QCOMPARE(submittedLocation, expectedCanonicalPath);
    QCOMPARE(coordinator.lastErrorKey(), QString{});
}

void MediaOpenCoordinatorTest::localFileBatchUsesBatchSubmissionInOrder()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString firstPath = directory.filePath(QStringLiteral("first.mp4"));
    const QString secondPath = directory.filePath(QStringLiteral("second.mp4"));
    for (const QString& path : {firstPath, secondPath}) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    int singleSubmissions = 0;
    int batchSubmissions = 0;
    QList<player::media::domain::MediaSource> submittedSources;
    MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource&) {
            ++singleSubmissions;
            return true;
        },
        [&](const QList<player::media::domain::MediaSource>& sources) {
            ++batchSubmissions;
            submittedSources = sources;
            return true;
        });

    QVERIFY(coordinator.openLocalFiles({
        QUrl::fromLocalFile(firstPath),
        QUrl::fromLocalFile(secondPath),
    }));
    QCOMPARE(singleSubmissions, 0);
    QCOMPARE(batchSubmissions, 1);
    QCOMPARE(submittedSources.size(), 2);
    QCOMPARE(submittedSources.at(0).location(), QFileInfo(firstPath).canonicalFilePath());
    QCOMPARE(submittedSources.at(1).location(), QFileInfo(secondPath).canonicalFilePath());
    QCOMPARE(
        static_cast<int>(submittedSources.at(0).kind()),
        static_cast<int>(player::media::domain::MediaSourceKind::LocalFile));
    QCOMPARE(
        static_cast<int>(submittedSources.at(1).kind()),
        static_cast<int>(player::media::domain::MediaSourceKind::LocalFile));
    QCOMPARE(coordinator.lastErrorKey(), QString{});
}

void MediaOpenCoordinatorTest::localFileBatchRejectsNonLocalUrl()
{
    int batchSubmissions = 0;
    MediaOpenCoordinator coordinator(
        [](const player::media::domain::MediaSource&) { return true; },
        [&](const QList<player::media::domain::MediaSource>&) {
            ++batchSubmissions;
            return true;
        });

    QVERIFY(!coordinator.openLocalFiles({
        QUrl(QStringLiteral("https://example.com/video.mp4")),
    }));
    QCOMPARE(batchSubmissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("not-local-file"));
}

void MediaOpenCoordinatorTest::validatedRemoteSourceUsesSameSubmissionBoundary()
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

    const auto source = player::media::domain::MediaSource::remoteUrl(
        QStringLiteral("https://example.com/video.mp4"));
    QVERIFY(coordinator.openSource(source));
    QCOMPARE(submissions, 1);
    QCOMPARE(
        static_cast<int>(submittedKind),
        static_cast<int>(player::media::domain::MediaSourceKind::RemoteUrl));
    QCOMPARE(submittedLocation, QStringLiteral("https://example.com/video.mp4"));
    QCOMPARE(coordinator.lastErrorKey(), QString{});
}

void MediaOpenCoordinatorTest::invalidSourceIsRejectedBeforeSubmission()
{
    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource&) {
            ++submissions;
            return true;
        });

    QVERIFY(!coordinator.openSource(player::media::domain::MediaSource::remoteUrl({})));
    QCOMPARE(submissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("submission-rejected"));
}

void MediaOpenCoordinatorTest::submissionRejectionIsReported()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString inputPath = directory.filePath(QStringLiteral("fixture.mp4"));
    QFile file(inputPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    int submissions = 0;
    MediaOpenCoordinator coordinator(
        [&submissions](const player::media::domain::MediaSource&) {
            ++submissions;
            return false;
        });
    QSignalSpy rejectedSpy(&coordinator, &MediaOpenCoordinator::openRejected);

    QVERIFY(!coordinator.openLocalFile(QUrl::fromLocalFile(inputPath)));
    QCOMPARE(submissions, 1);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("submission-rejected"));
    QCOMPARE(rejectedSpy.count(), 1);
}

void MediaOpenCoordinatorTest::batchUrlsAreValidatedInOrderAndSubmittedOnce()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString localPath = directory.filePath(QStringLiteral("local.mp4"));
    QFile file(localPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    int singleSubmissions = 0;
    int batchSubmissions = 0;
    QList<player::media::domain::MediaSource> submittedSources;
    MediaOpenCoordinator coordinator(
        [&](const player::media::domain::MediaSource&) {
            ++singleSubmissions;
            return true;
        },
        [&](const QList<player::media::domain::MediaSource>& sources) {
            ++batchSubmissions;
            submittedSources = sources;
            return true;
        });

    const QList<QUrl> urls{
        QUrl::fromLocalFile(localPath),
        QUrl(QStringLiteral("https://example.com/video.mp4")),
    };

    QVERIFY(coordinator.openSourceUrls(urls));
    QCOMPARE(singleSubmissions, 0);
    QCOMPARE(batchSubmissions, 1);
    QCOMPARE(submittedSources.size(), 2);
    QCOMPARE(submittedSources.at(0).location(), QFileInfo(localPath).canonicalFilePath());
    QCOMPARE(
        static_cast<int>(submittedSources.at(0).kind()),
        static_cast<int>(player::media::domain::MediaSourceKind::LocalFile));
    QCOMPARE(submittedSources.at(1).location(), QStringLiteral("https://example.com/video.mp4"));
    QCOMPARE(
        static_cast<int>(submittedSources.at(1).kind()),
        static_cast<int>(player::media::domain::MediaSourceKind::RemoteUrl));
    QCOMPARE(coordinator.lastErrorKey(), QString{});
}

void MediaOpenCoordinatorTest::invalidBatchUrlRejectsWholeBatchBeforeSubmission()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString localPath = directory.filePath(QStringLiteral("local.mp4"));
    QFile file(localPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    int batchSubmissions = 0;
    MediaOpenCoordinator coordinator(
        [](const player::media::domain::MediaSource&) { return true; },
        [&](const QList<player::media::domain::MediaSource>&) {
            ++batchSubmissions;
            return true;
        });

    QVERIFY(!coordinator.openSourceUrls({
        QUrl::fromLocalFile(localPath),
        QUrl(QStringLiteral("ftp://example.com/video.mp4")),
    }));
    QCOMPARE(batchSubmissions, 0);
    QCOMPARE(coordinator.lastErrorKey(), QStringLiteral("unsupported-url-scheme"));
}

} // namespace player::media::application

QTEST_GUILESS_MAIN(player::media::application::MediaOpenCoordinatorTest)
#include "media_open_coordinator_test.moc"
