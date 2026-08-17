#include "media/domain/media_source.h"
#include "playlist/domain/playlist.h"
#include "playlist/domain/playlist_navigation.h"

#include <QtTest>

namespace player::playlist::domain {
namespace {

media::domain::MediaSource localSource(const QString& path)
{
    return media::domain::MediaSource::localFile(path);
}

PlaylistNavigationDecision decisionFor(Playlist& playlist)
{
    return PlaylistNavigation::afterNaturalEnd(playlist);
}

int actionValue(PlaylistNavigationAction action)
{
    return static_cast<int>(action);
}

} // namespace

class PlaylistNavigationTest final : public QObject
{
    Q_OBJECT

private slots:
    void noCurrentProducesNoAction();
    void normalOrderAdvancesAndHonorsMove();
    void repeatModesHandleTailAndSingleItem();
    void shuffleDoesNotSilentlyFallBackToLinearOrder();
};

void PlaylistNavigationTest::noCurrentProducesNoAction()
{
    Playlist playlist;
    QVERIFY(playlist.append(localSource(QStringLiteral("C:/media/a.mp4"))).has_value());

    const auto decision = decisionFor(playlist);
    QCOMPARE(actionValue(decision.action), actionValue(PlaylistNavigationAction::None));
    QVERIFY(!decision.targetEntryId.isValid());
}

void PlaylistNavigationTest::normalOrderAdvancesAndHonorsMove()
{
    Playlist playlist;
    const auto first = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto second = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    const auto third = playlist.append(localSource(QStringLiteral("C:/media/c.mp4")));
    QVERIFY(first.has_value());
    QVERIFY(second.has_value());
    QVERIFY(third.has_value());
    QVERIFY(playlist.select(*first));

    auto decision = decisionFor(playlist);
    QCOMPARE(actionValue(decision.action), actionValue(PlaylistNavigationAction::SelectEntry));
    QCOMPARE(decision.targetEntryId.value(), second->value());

    QVERIFY(playlist.move(*third, 1));
    decision = decisionFor(playlist);
    QCOMPARE(actionValue(decision.action), actionValue(PlaylistNavigationAction::SelectEntry));
    QCOMPARE(decision.targetEntryId.value(), third->value());
}

void PlaylistNavigationTest::repeatModesHandleTailAndSingleItem()
{
    Playlist playlist;
    const auto first = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    const auto second = playlist.append(localSource(QStringLiteral("C:/media/b.mp4")));
    QVERIFY(first.has_value());
    QVERIFY(second.has_value());
    QVERIFY(playlist.select(*second));

    auto decision = decisionFor(playlist);
    QCOMPARE(actionValue(decision.action), actionValue(PlaylistNavigationAction::None));

    playlist.setRepeatMode(PlaylistRepeatMode::All);
    decision = decisionFor(playlist);
    QCOMPARE(actionValue(decision.action), actionValue(PlaylistNavigationAction::SelectEntry));
    QCOMPARE(decision.targetEntryId.value(), first->value());

    playlist.setRepeatMode(PlaylistRepeatMode::One);
    decision = decisionFor(playlist);
    QCOMPARE(actionValue(decision.action), actionValue(PlaylistNavigationAction::ReloadCurrent));
    QCOMPARE(decision.targetEntryId.value(), second->value());

    Playlist single;
    const auto only = single.append(localSource(QStringLiteral("C:/media/only.mp4")));
    QVERIFY(only.has_value());
    QVERIFY(single.select(*only));
    single.setRepeatMode(PlaylistRepeatMode::All);

    decision = decisionFor(single);
    QCOMPARE(actionValue(decision.action), actionValue(PlaylistNavigationAction::ReloadCurrent));
    QCOMPARE(decision.targetEntryId.value(), only->value());
}

void PlaylistNavigationTest::shuffleDoesNotSilentlyFallBackToLinearOrder()
{
    Playlist playlist;
    const auto first = playlist.append(localSource(QStringLiteral("C:/media/a.mp4")));
    QVERIFY(playlist.append(localSource(QStringLiteral("C:/media/b.mp4"))).has_value());
    QVERIFY(first.has_value());
    QVERIFY(playlist.select(*first));
    playlist.setShuffleEnabled(true);

    const auto decision = decisionFor(playlist);
    QCOMPARE(actionValue(decision.action), actionValue(PlaylistNavigationAction::None));
}

} // namespace player::playlist::domain

QTEST_GUILESS_MAIN(player::playlist::domain::PlaylistNavigationTest)
#include "playlist_navigation_test.moc"
