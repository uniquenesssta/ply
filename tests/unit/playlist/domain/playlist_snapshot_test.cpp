#include "media/domain/media_source.h"
#include "playlist/domain/playlist.h"
#include "playlist/domain/playlist_navigation.h"
#include "playlist/domain/playlist_snapshot.h"

#include <QtTest>

namespace player::playlist::domain {
namespace {

media::domain::MediaSource localSource(const QString& path)
{
    return media::domain::MediaSource::localFile(path);
}

int repeatValue(PlaylistRepeatMode mode)
{
    return static_cast<int>(mode);
}

} // namespace

class PlaylistSnapshotTest final : public QObject
{
    Q_OBJECT

private slots:
    void emptySnapshotHasNoDerivedIdentityOrNavigation();
    void snapshotKeepsStableOrderAndDerivedCurrentIndex();
    void snapshotCapturesShuffleBookkeepingWithoutBecomingWritableState();
    void navigationCapabilitiesFollowOrderRepeatAndShufflePolicy();
};

void PlaylistSnapshotTest::emptySnapshotHasNoDerivedIdentityOrNavigation()
{
    Playlist playlist;

    const PlaylistSnapshot snapshot = playlist.snapshot();
    const PlaylistNavigationCapabilities capabilities =
        PlaylistNavigation::capabilities(snapshot);

    QVERIFY(snapshot.empty());
    QCOMPARE(snapshot.size(), std::size_t{0});
    QVERIFY(snapshot.entries().empty());
    QVERIFY(!snapshot.currentId().has_value());
    QVERIFY(!snapshot.currentIndex().has_value());
    QCOMPARE(
        repeatValue(snapshot.repeatMode()),
        repeatValue(PlaylistRepeatMode::Off));
    QVERIFY(!snapshot.shuffleEnabled());
    QVERIFY(!snapshot.shuffleCycleInitialized());
    QVERIFY(snapshot.shuffleRemainingEntryIds().empty());
    QVERIFY(!capabilities.canNext);
    QVERIFY(!capabilities.canPrevious);
}

void PlaylistSnapshotTest::snapshotKeepsStableOrderAndDerivedCurrentIndex()
{
    Playlist playlist;
    const auto first = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto second = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    const auto third = playlist.append(localSource(QStringLiteral("C:/media/c.mp4")));
    QVERIFY(first.has_value());
    QVERIFY(second.has_value());
    QVERIFY(third.has_value());
    QVERIFY(playlist.select(*second));
    QVERIFY(playlist.move(*third, 0));
    playlist.setRepeatMode(PlaylistRepeatMode::All);

    const PlaylistSnapshot snapshot = playlist.snapshot();

    QCOMPARE(snapshot.size(), std::size_t{3});
    QCOMPARE(snapshot.entries().at(0).id().value(), third->value());
    QCOMPARE(snapshot.entries().at(1).id().value(), first->value());
    QCOMPARE(snapshot.entries().at(2).id().value(), second->value());
    QVERIFY(snapshot.currentId().has_value());
    QCOMPARE(snapshot.currentId()->value(), second->value());
    QVERIFY(snapshot.currentIndex().has_value());
    QCOMPARE(*snapshot.currentIndex(), std::size_t{2});
    QCOMPARE(
        repeatValue(snapshot.repeatMode()),
        repeatValue(PlaylistRepeatMode::All));

    // A snapshot is a detached observation. Later Playlist mutations cannot
    // rewrite an already published order/current projection.
    QVERIFY(playlist.move(*second, 0));
    QVERIFY(playlist.select(*first));
    QCOMPARE(snapshot.entries().at(2).id().value(), second->value());
    QCOMPARE(snapshot.currentId()->value(), second->value());
    QCOMPARE(*snapshot.currentIndex(), std::size_t{2});
}

void PlaylistSnapshotTest::snapshotCapturesShuffleBookkeepingWithoutBecomingWritableState()
{
    Playlist playlist;
    const auto first = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto second = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    const auto third = playlist.append(localSource(QStringLiteral("C:/media/c.mp4")));
    QVERIFY(first.has_value());
    QVERIFY(second.has_value());
    QVERIFY(third.has_value());
    QVERIFY(playlist.select(*first));
    playlist.setShuffleEnabled(true);

    const auto next = playlist.takeNextShuffledId(false);
    QVERIFY(next.has_value());
    QVERIFY(*next != *first);
    QVERIFY(playlist.select(*next));

    const PlaylistSnapshot snapshot = playlist.snapshot();
    QVERIFY(snapshot.shuffleEnabled());
    QVERIFY(snapshot.shuffleCycleInitialized());
    QCOMPARE(snapshot.shuffleRemainingEntryIds().size(), std::size_t{1});
    QVERIFY(snapshot.shuffleRemainingEntryIds().front() != *first);
    QVERIFY(snapshot.shuffleRemainingEntryIds().front() != *next);

    playlist.setShuffleEnabled(false);
    QVERIFY(snapshot.shuffleEnabled());
    QVERIFY(snapshot.shuffleCycleInitialized());
    QCOMPARE(snapshot.shuffleRemainingEntryIds().size(), std::size_t{1});
}

void PlaylistSnapshotTest::navigationCapabilitiesFollowOrderRepeatAndShufflePolicy()
{
    Playlist playlist;
    const auto first = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto second = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    const auto third = playlist.append(localSource(QStringLiteral("C:/media/c.mp4")));
    QVERIFY(first.has_value());
    QVERIFY(second.has_value());
    QVERIFY(third.has_value());

    QVERIFY(playlist.select(*first));
    auto capabilities = PlaylistNavigation::capabilities(playlist.snapshot());
    QVERIFY(capabilities.canNext);
    QVERIFY(!capabilities.canPrevious);

    QVERIFY(playlist.select(*second));
    capabilities = PlaylistNavigation::capabilities(playlist.snapshot());
    QVERIFY(capabilities.canNext);
    QVERIFY(capabilities.canPrevious);

    QVERIFY(playlist.select(*third));
    capabilities = PlaylistNavigation::capabilities(playlist.snapshot());
    QVERIFY(!capabilities.canNext);
    QVERIFY(capabilities.canPrevious);

    playlist.setRepeatMode(PlaylistRepeatMode::All);
    capabilities = PlaylistNavigation::capabilities(playlist.snapshot());
    QVERIFY(capabilities.canNext);
    QVERIFY(capabilities.canPrevious);

    playlist.setRepeatMode(PlaylistRepeatMode::Off);
    playlist.setShuffleEnabled(true);
    capabilities = PlaylistNavigation::capabilities(playlist.snapshot());
    QVERIFY(capabilities.canNext);
    QVERIFY(!capabilities.canPrevious);

    for (int index = 0; index < 2; ++index) {
        const auto next = playlist.takeNextShuffledId(false);
        QVERIFY(next.has_value());
        QVERIFY(playlist.select(*next));
    }

    capabilities = PlaylistNavigation::capabilities(playlist.snapshot());
    QVERIFY(!capabilities.canNext);
    QVERIFY(!capabilities.canPrevious);

    playlist.setRepeatMode(PlaylistRepeatMode::All);
    capabilities = PlaylistNavigation::capabilities(playlist.snapshot());
    QVERIFY(capabilities.canNext);
    QVERIFY(!capabilities.canPrevious);
}

} // namespace player::playlist::domain

QTEST_GUILESS_MAIN(player::playlist::domain::PlaylistSnapshotTest)
#include "playlist_snapshot_test.moc"
