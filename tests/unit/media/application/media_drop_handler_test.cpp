#include "media/application/drop/media_drop_handler.h"
#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"

#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

namespace player::media::application {

class MediaDropHandlerTest final : public QObject
{
    Q_OBJECT

private slots:
    void singleLocalFileUsesExistingCoordinator();
    void multipleLocalFilesPreserveOrderAndSubmitOneBatch();
    void directoryIsRejected();
    void remoteUrlUsesUrlWorkflow();
    void multipleSourcesPreserveOrderAndSubmitOneBatch();
    void unsupportedSchemeIsRejected();
};

void MediaDropHandlerTest::singleLocalFileUsesExistingCoordinator()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("sample.mp4"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("media");
    file.close();

    int submissions = 0;
    MediaOpenCoordinator coordinator([&](const player::media::domain::MediaSource&) {
        ++submissions;
        return true;
    });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaDropHandler handler(coordinator, urlWorkflow);

    const QList<QUrl> urls{QUrl::fromLocalFile(path)};
    QVERIFY(handler.canHandle(urls));
    QVERIFY(handler.handleDrop(urls));
    QCOMPARE(submissions, 1);
    QCOMPARE(handler.lastOutcomeKey(), QStringLiteral("opened"));
}

void MediaDropHandlerTest::multipleLocalFilesPreserveOrderAndSubmitOneBatch()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QList<QUrl> urls;
    for (const QString& name : {QStringLiteral("b.mp4"), QStringLiteral("a.mp4")}) {
        const QString path = directory.filePath(name);
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("media");
        file.close();
        urls.push_back(QUrl::fromLocalFile(path));
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
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaDropHandler handler(coordinator, urlWorkflow);
    QSignalSpy deferredSpy(&handler, &MediaDropHandler::dropDeferred);

    QVERIFY(handler.canHandle(urls));
    QVERIFY(handler.handleDrop(urls));
    QCOMPARE(singleSubmissions, 0);
    QCOMPARE(batchSubmissions, 1);
    QCOMPARE(handler.lastOutcomeKey(), QStringLiteral("opened"));
    QCOMPARE(deferredSpy.count(), 0);
    QCOMPARE(submittedSources.size(), 2);
    QCOMPARE(submittedSources.at(0).location(), QFileInfo(urls.at(0).toLocalFile()).canonicalFilePath());
    QCOMPARE(submittedSources.at(1).location(), QFileInfo(urls.at(1).toLocalFile()).canonicalFilePath());

    const QVariantList ordered = handler.orderedSourceUrls();
    QCOMPARE(ordered.size(), 2);
    QCOMPARE(ordered.at(0).toUrl(), urls.at(0));
    QCOMPARE(ordered.at(1).toUrl(), urls.at(1));
}

void MediaDropHandlerTest::directoryIsRejected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    MediaOpenCoordinator coordinator([](const player::media::domain::MediaSource&) { return true; });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaDropHandler handler(coordinator, urlWorkflow);

    const QList<QUrl> urls{QUrl::fromLocalFile(directory.path())};
    QVERIFY(!handler.canHandle(urls));
    QVERIFY(!handler.handleDrop(urls));
    QCOMPARE(handler.lastOutcomeKey(), QStringLiteral("directory-rejected"));
}

void MediaDropHandlerTest::remoteUrlUsesUrlWorkflow()
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
    MediaDropHandler handler(coordinator, urlWorkflow);

    const QList<QUrl> urls{QUrl(QStringLiteral("https://example.com/video.mp4"))};
    QVERIFY(handler.canHandle(urls));
    QVERIFY(handler.handleDrop(urls));
    QCOMPARE(submissions, 1);
    QCOMPARE(
        static_cast<int>(submittedKind),
        static_cast<int>(player::media::domain::MediaSourceKind::RemoteUrl));
    QCOMPARE(submittedLocation, QStringLiteral("https://example.com/video.mp4"));
    QCOMPARE(handler.lastOutcomeKey(), QStringLiteral("opened"));
}

void MediaDropHandlerTest::multipleSourcesPreserveOrderAndSubmitOneBatch()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString localPath = directory.filePath(QStringLiteral("local.mp4"));
    QFile file(localPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("media");
    file.close();

    const QList<QUrl> urls{
        QUrl(QStringLiteral("https://example.com/b.mp4")),
        QUrl::fromLocalFile(localPath),
    };

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
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaDropHandler handler(coordinator, urlWorkflow);
    QSignalSpy deferredSpy(&handler, &MediaDropHandler::dropDeferred);

    QVERIFY(handler.canHandle(urls));
    QVERIFY(handler.handleDrop(urls));
    QCOMPARE(singleSubmissions, 0);
    QCOMPARE(batchSubmissions, 1);
    QCOMPARE(handler.lastOutcomeKey(), QStringLiteral("opened"));
    QCOMPARE(deferredSpy.count(), 0);
    QCOMPARE(submittedSources.size(), 2);
    QCOMPARE(submittedSources.at(0).location(), QStringLiteral("https://example.com/b.mp4"));
    QCOMPARE(submittedSources.at(1).location(), QFileInfo(localPath).canonicalFilePath());

    const QVariantList ordered = handler.orderedSourceUrls();
    QCOMPARE(ordered.size(), 2);
    QCOMPARE(ordered.at(0).toUrl(), urls.at(0));
    QCOMPARE(ordered.at(1).toUrl(), urls.at(1));
}

void MediaDropHandlerTest::unsupportedSchemeIsRejected()
{
    MediaOpenCoordinator coordinator([](const player::media::domain::MediaSource&) { return true; });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaDropHandler handler(coordinator, urlWorkflow);

    const QList<QUrl> urls{QUrl(QStringLiteral("ftp://example.com/video.mp4"))};
    QVERIFY(!handler.canHandle(urls));
    QVERIFY(!handler.handleDrop(urls));
    QCOMPARE(handler.lastOutcomeKey(), QStringLiteral("unsupported"));
}

} // namespace player::media::application

QTEST_MAIN(player::media::application::MediaDropHandlerTest)
#include "media_drop_handler_test.moc"
