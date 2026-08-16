#include "media/domain/media_source.h"
#include "playlist/domain/playlist.h"

#include <QtTest>

namespace player::playlist::domain {
namespace {

media::domain::MediaSource localSource(const QString& path)
{
    return media::domain::MediaSource::localFile(path);
}

} // namespace

class PlaylistTest final : public QObject
{
    Q_OBJECT

private slots:
    void emptyPlaylistHasNoCurrentAndDefaultModes();
    void appendPreservesOrderAndAllocatesStableIds();
    void invalidSourceIsRejectedWithoutConsumingId();
    void duplicateSourcesReceiveDistinctIds();
    void selectRequiresExistingEntry();
    void removeNonCurrentPreservesCurrent();
    void removeCurrentClearsCurrentWithoutDanglingId();
    void clearRemovesQueueButDoesNotReuseIdsOrResetModes();
};

void PlaylistTest::emptyPlaylistHasNoCurrentAndDefaultModes()
{
    const Playlist playlist;

    QVERIFY(playlist.empty());
    QCOMPARE(playlist.size(), std::size_t{0});
    QVERIFY(!playlist.currentId().has_value());
    QVERIFY(playlist.currentEntry() == nullptr);
    QCOMPARE(
        static_cast<int>(playlist.repeatMode()),
        static_cast<int>(PlaylistRepeatMode::Off));
    QVERIFY(!playlist.shuffleEnabled());
}

void PlaylistTest::appendPreservesOrderAndAllocatesStableIds()
{
    Playlist playlist;

    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto secondId = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));

    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(firstId->isValid());
    QVERIFY(secondId->isValid());
    QVERIFY(*firstId != *secondId);
    QCOMPARE(playlist.size(), std::size_t{2});
    QCOMPARE(playlist.entries().at(0).id().value(), firstId->value());
    QCOMPARE(playlist.entries().at(0).source().location(), QStringLiteral("C:/media/a.mp4"));
    QCOMPARE(playlist.entries().at(1).id().value(), secondId->value());
    QCOMPARE(playlist.entries().at(1).source().location(), QStringLiteral("C:/media/b.mp4"));
    QVERIFY(!playlist.currentId().has_value());
}

void PlaylistTest::invalidSourceIsRejectedWithoutConsumingId()
{
    Playlist playlist;

    const auto rejected = playlist.append(localSource(QString{}));
    const auto accepted = playlist.append(localSource(QStringLiteral("C:/media/valid.mp4")));

    QVERIFY(!rejected.has_value());
    QVERIFY(accepted.has_value());
    QCOMPARE(accepted->value(), quint64{1});
    QCOMPARE(playlist.size(), std::size_t{1});
}

void PlaylistTest::duplicateSourcesReceiveDistinctIds()
{
    Playlist playlist;
    const auto source = localSource(QStringLiteral("C:/media/repeat.mp4"));

    const auto firstId = playlist.append(source);
    const auto secondId = playlist.append(source);

    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(*firstId != *secondId);
    QCOMPARE(playlist.size(), std::size_t{2});
}

void PlaylistTest::selectRequiresExistingEntry()
{
    Playlist playlist;
    const auto id = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    QVERIFY(id.has_value());

    QVERIFY(!playlist.select(PlaylistEntryId{}));
    QVERIFY(!playlist.select(PlaylistEntryId{9999}));
    QVERIFY(!playlist.currentId().has_value());

    QVERIFY(playlist.select(*id));
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), id->value());
    QVERIFY(playlist.currentEntry() != nullptr);
    QCOMPARE(playlist.currentEntry()->id().value(), id->value());
}

void PlaylistTest::removeNonCurrentPreservesCurrent()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto secondId = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(playlist.select(*secondId));

    QVERIFY(playlist.remove(*firstId));
    QCOMPARE(playlist.size(), std::size_t{1});
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), secondId->value());
    QVERIFY(playlist.find(*firstId) == nullptr);
    QVERIFY(playlist.find(*secondId) != nullptr);
}

void PlaylistTest::removeCurrentClearsCurrentWithoutDanglingId()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto secondId = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(playlist.select(*firstId));

    QVERIFY(playlist.remove(*firstId));
    QVERIFY(!playlist.currentId().has_value());
    QVERIFY(playlist.currentEntry() == nullptr);
    QCOMPARE(playlist.size(), std::size_t{1});
    QVERIFY(playlist.find(*secondId) != nullptr);
    QVERIFY(!playlist.remove(*firstId));
}

void PlaylistTest::clearRemovesQueueButDoesNotReuseIdsOrResetModes()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto secondId = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(playlist.select(*secondId));

    playlist.setRepeatMode(PlaylistRepeatMode::All);
    playlist.setShuffleEnabled(true);
    playlist.clear();

    QVERIFY(playlist.empty());
    QVERIFY(!playlist.currentId().has_value());
    QCOMPARE(
        static_cast<int>(playlist.repeatMode()),
        static_cast<int>(PlaylistRepeatMode::All));
    QVERIFY(playlist.shuffleEnabled());

    const auto thirdId = playlist.append(localSource(QStringLiteral("C:/media/c.mp4")));
    QVERIFY(thirdId.has_value());
    QVERIFY(*thirdId != *firstId);
    QVERIFY(*thirdId != *secondId);
    QCOMPARE(thirdId->value(), quint64{3});
}

} // namespace player::playlist::domain

QTEST_GUILESS_MAIN(player::playlist::domain::PlaylistTest)
#include "playlist_test.moc"
