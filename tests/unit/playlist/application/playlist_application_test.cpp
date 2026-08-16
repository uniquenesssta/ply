#include "media/domain/media_source.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/application/playlist_mutation.h"
#include "playlist/domain/playlist.h"

#include <QList>
#include <QtTest>

namespace player::playlist::application {
namespace {

media::domain::MediaSource localSource(const QString& path)
{
    return media::domain::MediaSource::localFile(path);
}

media::domain::MediaSource remoteSource(const QString& url)
{
    return media::domain::MediaSource::remoteUrl(url);
}

} // namespace

class PlaylistApplicationTest final : public QObject
{
    Q_OBJECT

private slots:
    void mutationBatchIsAllOrNothing();
    void openBatchPreservesOrderAndLoadsOnlyFirst();
    void rejectedLoadRollsBackBatchAndRestoresCurrent();
    void rejectedSelectionRestoresPreviousCurrent();
    void removeAndMoveGoThroughControllerWithoutRemovingCurrent();
};

void PlaylistApplicationTest::mutationBatchIsAllOrNothing()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);

    const auto result = mutation.appendAll({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QString{}),
        localSource(QStringLiteral("C:/media/b.mp4")),
    });

    QVERIFY(!result.has_value());
    QVERIFY(playlist.empty());
    QVERIFY(!playlist.currentId().has_value());
}

void PlaylistApplicationTest::openBatchPreservesOrderAndLoadsOnlyFirst()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    int submissions = 0;
    QString submittedLocation;
    PlaylistController controller(
        playlist,
        mutation,
        [&](const media::domain::MediaSource& source) {
            ++submissions;
            submittedLocation = source.location();
            return true;
        });

    const QList<media::domain::MediaSource> sources{
        localSource(QStringLiteral("C:/media/b.mp4")),
        remoteSource(QStringLiteral("https://example.com/a.mp4")),
    };

    QVERIFY(controller.openSources(sources));
    QCOMPARE(submissions, 1);
    QCOMPARE(submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(playlist.size(), std::size_t{2});
    QCOMPARE(playlist.entries().at(0).source().location(), sources.at(0).location());
    QCOMPARE(playlist.entries().at(1).source().location(), sources.at(1).location());
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), playlist.entries().at(0).id().value());
}

void PlaylistApplicationTest::rejectedLoadRollsBackBatchAndRestoresCurrent()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource& source) {
            return source.location() != QStringLiteral("C:/media/reject.mp4");
        });

    QVERIFY(controller.openSource(localSource(QStringLiteral("C:/media/existing.mp4"))));
    const auto previousCurrent = playlist.currentId();
    QVERIFY(previousCurrent.has_value());
    QCOMPARE(playlist.size(), std::size_t{1});

    QVERIFY(!controller.openSources({
        localSource(QStringLiteral("C:/media/reject.mp4")),
        localSource(QStringLiteral("C:/media/queued.mp4")),
    }));

    QCOMPARE(playlist.size(), std::size_t{1});
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), previousCurrent->value());
    QCOMPARE(
        playlist.currentEntry()->source().location(),
        QStringLiteral("C:/media/existing.mp4"));
}

void PlaylistApplicationTest::rejectedSelectionRestoresPreviousCurrent()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource& source) {
            return source.location() != QStringLiteral("C:/media/b.mp4");
        });

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
    }));
    const domain::PlaylistEntryId firstId = playlist.entries().at(0).id();
    const domain::PlaylistEntryId secondId = playlist.entries().at(1).id();
    QCOMPARE(playlist.currentId()->value(), firstId.value());

    QVERIFY(!controller.selectEntry(secondId.value()));
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), firstId.value());
}

void PlaylistApplicationTest::removeAndMoveGoThroughControllerWithoutRemovingCurrent()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource&) { return true; });

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
        localSource(QStringLiteral("C:/media/c.mp4")),
    }));

    const domain::PlaylistEntryId currentId = playlist.entries().at(0).id();
    const domain::PlaylistEntryId secondId = playlist.entries().at(1).id();
    const domain::PlaylistEntryId thirdId = playlist.entries().at(2).id();

    QVERIFY(!controller.removeEntry(currentId.value()));
    QCOMPARE(playlist.size(), std::size_t{3});

    QVERIFY(controller.moveEntry(thirdId.value(), 0));
    QCOMPARE(playlist.entries().at(0).id().value(), thirdId.value());
    QCOMPARE(playlist.currentId()->value(), currentId.value());

    QVERIFY(controller.removeEntry(secondId.value()));
    QCOMPARE(playlist.size(), std::size_t{2});
    QVERIFY(playlist.find(secondId) == nullptr);
    QCOMPARE(playlist.currentId()->value(), currentId.value());
}

} // namespace player::playlist::application

QTEST_GUILESS_MAIN(player::playlist::application::PlaylistApplicationTest)
#include "playlist_application_test.moc"
