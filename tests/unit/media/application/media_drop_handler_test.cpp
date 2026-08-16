#include "media/application/drop/media_drop_handler.h"
#include "media/application/open/media_open_coordinator.h"
#include "media/application/open/url_open_workflow.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

namespace player::media::application {

class MediaDropHandlerTest final : public QObject
{
    Q_OBJECT

private slots:
    void singleLocalFileUsesExistingCoordinator();
    void multipleLocalFilesPreserveOrderWithoutRepeatedLoad();
    void directoryIsRejected();
    void remoteUrlUsesUrlWorkflow();
    void multipleSourcesPreserveOrderWithoutRepeatedLoad();
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

void MediaDropHandlerTest::multipleLocalFilesPreserveOrderWithoutRepeatedLoad()
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

    int submissions = 0;
    MediaOpenCoordinator coordinator([&](const player::media::domain::MediaSource&) {
        ++submissions;
        return true;
    });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaDropHandler handler(coordinator, urlWorkflow);
    QSignalSpy deferredSpy(&handler, &MediaDropHandler::dropDeferred);

    QVERIFY(handler.canHandle(urls));
    QVERIFY(!handler.handleDrop(urls));
    QCOMPARE(submissions, 0);
    QCOMPARE(handler.lastOutcomeKey(), QStringLiteral("multiple-files-deferred"));
    QCOMPARE(deferredSpy.count(), 1);

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

void MediaDropHandlerTest::multipleSourcesPreserveOrderWithoutRepeatedLoad()
{
    const QList<QUrl> urls{
        QUrl(QStringLiteral("https://example.com/b.mp4")),
        QUrl(QStringLiteral("https://example.com/a.mp4"))};

    int submissions = 0;
    MediaOpenCoordinator coordinator([&](const player::media::domain::MediaSource&) {
        ++submissions;
        return true;
    });
    UrlOpenWorkflow urlWorkflow(coordinator);
    MediaDropHandler handler(coordinator, urlWorkflow);
    QSignalSpy deferredSpy(&handler, &MediaDropHandler::dropDeferred);

    QVERIFY(handler.canHandle(urls));
    QVERIFY(!handler.handleDrop(urls));
    QCOMPARE(submissions, 0);
    QCOMPARE(handler.lastOutcomeKey(), QStringLiteral("multiple-sources-deferred"));
    QCOMPARE(deferredSpy.count(), 1);

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
