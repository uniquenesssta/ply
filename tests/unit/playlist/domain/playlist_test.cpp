#include "media/domain/media_source.h"
#include "playlist/domain/playlist.h"

#include <QtTest>

#include <utility>

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
    void insertPreservesCurrentAndStableIdentity();
    void insertRejectsInvalidInputWithoutConsumingId();
    void selectRequiresExistingEntry();
    void movePreservesStableIdsAndCurrent();
    void moveRejectsMissingEntryAndOutOfRangeTarget();
    void clearCurrentKeepsQueue();
    void removeNonCurrentPreservesCurrent();
    void removeCurrentClearsCurrentWithoutDanglingId();
    void removeCurrentAndSelectIsAtomicForValidReplacement();
    void removeCurrentAndSelectRejectsInvalidTransitionWithoutMutation();
    void preparedReplacementCommitsAtomicallyAndPreservesModes();
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

void PlaylistTest::insertPreservesCurrentAndStableIdentity()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    QVERIFY(firstId.has_value());
    QVERIFY(playlist.select(*firstId));

    const auto inserted = playlist.insert(
        localSource(QStringLiteral("C:/media/b.mp4")),
        0);
    QVERIFY(inserted.has_value());
    QCOMPARE(inserted->value(), quint64{2});
    QCOMPARE(playlist.entries().at(0).id().value(), inserted->value());
    QCOMPARE(playlist.entries().at(1).id().value(), firstId->value());
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), firstId->value());

    const PlaylistSnapshot snapshot = playlist.snapshot();
    QVERIFY(snapshot.currentIndex().has_value());
    QCOMPARE(*snapshot.currentIndex(), std::size_t{1});

    const auto tail = playlist.insert(
        localSource(QStringLiteral("C:/media/c.mp4")),
        playlist.size());
    QVERIFY(tail.has_value());
    QCOMPARE(tail->value(), quint64{3});
    QCOMPARE(playlist.entries().back().id().value(), tail->value());
    QCOMPARE(playlist.currentId()->value(), firstId->value());
}

void PlaylistTest::insertRejectsInvalidInputWithoutConsumingId()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    QVERIFY(firstId.has_value());

    QVERIFY(!playlist.insert(localSource(QString{}), 0).has_value());
    QVERIFY(!playlist.insert(
        localSource(QStringLiteral("C:/media/out-of-range.mp4")),
        2).has_value());

    const auto inserted = playlist.insert(
        localSource(QStringLiteral("C:/media/b.mp4")),
        1);
    QVERIFY(inserted.has_value());
    QCOMPARE(inserted->value(), quint64{2});
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

void PlaylistTest::movePreservesStableIdsAndCurrent()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto secondId = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    const auto thirdId = playlist.append(localSource(QStringLiteral("C:/media/c.mp4")));
    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(thirdId.has_value());
    QVERIFY(playlist.select(*secondId));

    QVERIFY(playlist.move(*thirdId, 0));
    QCOMPARE(playlist.entries().at(0).id().value(), thirdId->value());
    QCOMPARE(playlist.entries().at(1).id().value(), firstId->value());
    QCOMPARE(playlist.entries().at(2).id().value(), secondId->value());
    QCOMPARE(playlist.currentId()->value(), secondId->value());

    QVERIFY(playlist.move(*thirdId, 2));
    QCOMPARE(playlist.entries().at(0).id().value(), firstId->value());
    QCOMPARE(playlist.entries().at(1).id().value(), secondId->value());
    QCOMPARE(playlist.entries().at(2).id().value(), thirdId->value());
    QCOMPARE(playlist.currentId()->value(), secondId->value());
}

void PlaylistTest::moveRejectsMissingEntryAndOutOfRangeTarget()
{
    Playlist playlist;
    const auto id = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    QVERIFY(id.has_value());

    QVERIFY(!playlist.move(PlaylistEntryId{9999}, 0));
    QVERIFY(!playlist.move(*id, 1));
    QCOMPARE(playlist.entries().at(0).id().value(), id->value());
}

void PlaylistTest::clearCurrentKeepsQueue()
{
    Playlist playlist;
    const auto id = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    QVERIFY(id.has_value());
    QVERIFY(playlist.select(*id));

    playlist.clearCurrent();

    QCOMPARE(playlist.size(), std::size_t{1});
    QVERIFY(!playlist.currentId().has_value());
    QVERIFY(playlist.find(*id) != nullptr);
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

void PlaylistTest::removeCurrentAndSelectIsAtomicForValidReplacement()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto secondId = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    const auto thirdId = playlist.append(localSource(QStringLiteral("C:/media/c.mp4")));
    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(thirdId.has_value());
    QVERIFY(playlist.select(*secondId));
    playlist.setShuffleEnabled(true);

    QVERIFY(playlist.removeCurrentAndSelect(*secondId, *thirdId));

    QCOMPARE(playlist.size(), std::size_t{2});
    QVERIFY(playlist.find(*secondId) == nullptr);
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), thirdId->value());
    QCOMPARE(playlist.entries().at(0).id().value(), firstId->value());
    QCOMPARE(playlist.entries().at(1).id().value(), thirdId->value());
}

void PlaylistTest::removeCurrentAndSelectRejectsInvalidTransitionWithoutMutation()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto secondId = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(playlist.select(*firstId));

    QVERIFY(!playlist.removeCurrentAndSelect(*secondId, *firstId));
    QVERIFY(!playlist.removeCurrentAndSelect(*firstId, PlaylistEntryId{9999}));
    QVERIFY(!playlist.removeCurrentAndSelect(*firstId, *firstId));

    QCOMPARE(playlist.size(), std::size_t{2});
    QVERIFY(playlist.find(*firstId) != nullptr);
    QVERIFY(playlist.find(*secondId) != nullptr);
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), firstId->value());
}

void PlaylistTest::preparedReplacementCommitsAtomicallyAndPreservesModes()
{
    Playlist playlist;
    const auto firstId = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto secondId = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    QVERIFY(firstId.has_value());
    QVERIFY(secondId.has_value());
    QVERIFY(playlist.select(*secondId));
    playlist.setRepeatMode(PlaylistRepeatMode::All);
    playlist.setShuffleEnabled(true);
    QVERIFY(playlist.takeNextShuffledId(false).has_value());
    QVERIFY(playlist.snapshot().shuffleCycleInitialized());

    auto replacement = playlist.prepareReplacement({
        localSource(QStringLiteral("C:/media/c.mp4")),
        localSource(QStringLiteral("C:/media/d.mp4")),
    });
    QVERIFY(replacement.has_value());

    // Preparation is detached: the authoritative queue/current remains intact
    // until the application has accepted the target load and commits it.
    QCOMPARE(playlist.size(), std::size_t{2});
    QCOMPARE(playlist.currentId()->value(), secondId->value());
    QCOMPARE(playlist.entries().at(0).id().value(), firstId->value());
    QCOMPARE(playlist.entries().at(1).id().value(), secondId->value());
    QCOMPARE(replacement->entries().at(0).id().value(), quint64{3});
    QCOMPARE(replacement->entries().at(1).id().value(), quint64{4});
    QCOMPARE(replacement->currentId().value(), quint64{3});

    playlist.commitReplacement(std::move(*replacement));

    QCOMPARE(playlist.size(), std::size_t{2});
    QCOMPARE(playlist.entries().at(0).source().location(), QStringLiteral("C:/media/c.mp4"));
    QCOMPARE(playlist.entries().at(1).source().location(), QStringLiteral("C:/media/d.mp4"));
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), quint64{3});
    QCOMPARE(
        static_cast<int>(playlist.repeatMode()),
        static_cast<int>(PlaylistRepeatMode::All));
    QVERIFY(playlist.shuffleEnabled());
    QVERIFY(!playlist.snapshot().shuffleCycleInitialized());

    const auto nextId = playlist.append(localSource(QStringLiteral("C:/media/e.mp4")));
    QVERIFY(nextId.has_value());
    QCOMPARE(nextId->value(), quint64{5});
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
