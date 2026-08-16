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
    void validatedRemoteSourceUsesSameSubmissionBoundary();
    void invalidSourceIsRejectedBeforeSubmission();
    void submissionRejectionIsReported();
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

} // namespace player::media::application

QTEST_GUILESS_MAIN(player::media::application::MediaOpenCoordinatorTest)
#include "media_open_coordinator_test.moc"
