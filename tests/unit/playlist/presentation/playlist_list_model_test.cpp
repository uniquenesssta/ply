#include "media/domain/media_source.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/application/playlist_mutation.h"
#include "playlist/domain/playlist.h"
#include "playlist/presentation/playlist_list_model.h"

#include <QtTest>

namespace player::playlist::presentation {
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

class PlaylistListModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void startsEmptyAndReadOnly();
    void projectsOrderIdentityTitleAndCurrent();
    void followsCommittedSelectAndMove();
};

void PlaylistListModelTest::startsEmptyAndReadOnly()
{
    domain::Playlist playlist;
    application::PlaylistMutation mutation(playlist);
    application::PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource&) { return true; });
    PlaylistListModel model(controller);

    QCOMPARE(model.count(), 0);
    QCOMPARE(model.currentPosition(), 0);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.flags(QModelIndex{}), Qt::ItemFlags{});
}

void PlaylistListModelTest::projectsOrderIdentityTitleAndCurrent()
{
    domain::Playlist playlist;
    application::PlaylistMutation mutation(playlist);
    application::PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource&) { return true; });
    PlaylistListModel model(controller);

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/local sample.mp4")),
        remoteSource(QStringLiteral("https://example.com/video/remote.mp4")),
    }));

    QCOMPARE(model.count(), 2);
    QCOMPARE(model.currentPosition(), 1);
    const QModelIndex first = model.index(0, 0);
    const QModelIndex second = model.index(1, 0);
    QVERIFY(first.isValid());
    QVERIFY(second.isValid());
    QVERIFY(!(model.flags(first) & Qt::ItemIsEditable));
    QCOMPARE(
        model.data(first, PlaylistListModel::EntryIdRole).toULongLong(),
        playlist.entries().at(0).id().value());
    QCOMPARE(
        model.data(first, PlaylistListModel::DisplayTitleRole).toString(),
        QStringLiteral("local sample.mp4"));
    QVERIFY(model.data(first, PlaylistListModel::CurrentRole).toBool());
    QCOMPARE(
        model.data(second, PlaylistListModel::DisplayTitleRole).toString(),
        QStringLiteral("remote.mp4"));
    QVERIFY(!model.data(second, PlaylistListModel::CurrentRole).toBool());
    QVERIFY(!model.setData(first, QStringLiteral("mutate"), PlaylistListModel::DisplayTitleRole));
}

void PlaylistListModelTest::followsCommittedSelectAndMove()
{
    domain::Playlist playlist;
    application::PlaylistMutation mutation(playlist);
    application::PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource&) { return true; });
    PlaylistListModel model(controller);

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
    }));
    const quint64 firstId = playlist.entries().at(0).id().value();
    const quint64 secondId = playlist.entries().at(1).id().value();

    QVERIFY(controller.selectEntry(secondId));
    QCOMPARE(model.currentPosition(), 2);
    QVERIFY(!model.data(model.index(0, 0), PlaylistListModel::CurrentRole).toBool());
    QVERIFY(model.data(model.index(1, 0), PlaylistListModel::CurrentRole).toBool());

    QVERIFY(controller.moveEntry(secondId, 0));
    QCOMPARE(model.currentPosition(), 1);
    QCOMPARE(
        model.data(model.index(0, 0), PlaylistListModel::EntryIdRole).toULongLong(),
        secondId);
    QCOMPARE(
        model.data(model.index(1, 0), PlaylistListModel::EntryIdRole).toULongLong(),
        firstId);
    QVERIFY(model.data(model.index(0, 0), PlaylistListModel::CurrentRole).toBool());
}

} // namespace player::playlist::presentation

QTEST_GUILESS_MAIN(player::playlist::presentation::PlaylistListModelTest)
#include "playlist_list_model_test.moc"
