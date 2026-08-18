#include "media/domain/media_source.h"
#include "playlist/application/playlist_advance_arbiter.h"
#include "playlist/application/playlist_auto_advance.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/application/playlist_mutation.h"
#include "playlist/domain/playlist.h"

#include <QtTest>

namespace player::playlist::application {
namespace {

media::domain::MediaSource localSource(const QString& path)
{
    return media::domain::MediaSource::localFile(path);
}

struct Fixture final
{
    domain::Playlist playlist;
    PlaylistMutation mutation{playlist};
    PlaylistAdvanceArbiter advanceArbiter;
    int submissions = 0;
    QString submittedLocation;
    QString rejectedLocation;
    PlaylistController controller{
        playlist,
        mutation,
        [this](const media::domain::MediaSource& source) {
            ++submissions;
            submittedLocation = source.location();
            const bool accepted = source.location() != rejectedLocation;
            if (accepted) {
                // Production PlaybackComposition notifies this same arbiter
                // after an accepted LoadMedia command enters the command bus.
                advanceArbiter.suppressObservedGeneration();
            }
            return accepted;
        },
        PlaylistController::SubmitMediaStop{},
        nullptr,
        &advanceArbiter};
    PlaylistAutoAdvance autoAdvance{playlist, controller, advanceArbiter};
};

void openTwo(Fixture& fixture)
{
    QVERIFY(fixture.controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
    }));
    fixture.submissions = 0;
    fixture.submittedLocation.clear();
}

void openThree(Fixture& fixture)
{
    QVERIFY(fixture.controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
        localSource(QStringLiteral("C:/media/c.mp4")),
    }));
    fixture.submissions = 0;
    fixture.submittedLocation.clear();
}

void openFour(Fixture& fixture)
{
    QVERIFY(fixture.controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
        localSource(QStringLiteral("C:/media/c.mp4")),
        localSource(QStringLiteral("C:/media/d.mp4")),
    }));
    fixture.submissions = 0;
    fixture.submittedLocation.clear();
}

} // namespace

class PlaylistAutoAdvanceTest final : public QObject
{
    Q_OBJECT

private slots:
    void naturalEndAdvancesExactlyOnce();
    void manualNextSuppressesLateEndAndNewGenerationRearms();
    void eofFirstBlocksManualNextUntilNewGeneration();
    void rapidManualNextAllowsOnlyOneAdvancePerObservedGeneration();
    void acceptedStopSuppressesLateNaturalEnd();
    void rejectedManualNextReleasesClaimForNaturalEnd();
    void repeatOneReloadsCurrent();
    void repeatAllWrapsTail();
    void shuffleAdvancesThroughControllerWithoutImmediateReplay();
    void failureSkipsNextAndDoesNotRetryOrWrap();
    void shuffleFailureDoesNotRestartRepeatAllCycle();
    void rejectedAdvanceRestoresCurrentAndDoesNotRetrySameGeneration();
    void newGenerationCanAdvanceAfterPreviousEnd();
};

void PlaylistAutoAdvanceTest::naturalEndAdvancesExactlyOnce()
{
    Fixture fixture;
    openTwo(fixture);
    const auto secondId = fixture.playlist.entries().at(1).id();

    fixture.autoAdvance.acceptPlaybackState(1, false);
    QCOMPARE(fixture.submissions, 0);

    fixture.autoAdvance.acceptPlaybackState(1, true);
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    fixture.autoAdvance.acceptPlaybackState(1, false);
    fixture.autoAdvance.acceptPlaybackState(1, true);
    QCOMPARE(fixture.submissions, 1);
}

void PlaylistAutoAdvanceTest::manualNextSuppressesLateEndAndNewGenerationRearms()
{
    Fixture fixture;
    openThree(fixture);
    const auto secondId = fixture.playlist.entries().at(1).id();
    const auto thirdId = fixture.playlist.entries().at(2).id();

    fixture.autoAdvance.acceptPlaybackState(30, false);
    QVERIFY(fixture.controller.nextEntry());
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    // The old generation may still publish Ended before the queued B load has
    // reached PlaybackSession. The already accepted manual Next owns A's
    // transition, so A must not auto-advance B again to C.
    fixture.autoAdvance.acceptPlaybackState(30, true);
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    fixture.autoAdvance.acceptPlaybackState(31, false);
    fixture.autoAdvance.acceptPlaybackState(30, true);
    QCOMPARE(fixture.submissions, 1);

    fixture.autoAdvance.acceptPlaybackState(31, true);
    QCOMPARE(fixture.submissions, 2);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/c.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), thirdId.value());
}

void PlaylistAutoAdvanceTest::eofFirstBlocksManualNextUntilNewGeneration()
{
    Fixture fixture;
    openThree(fixture);
    const auto secondId = fixture.playlist.entries().at(1).id();
    const auto thirdId = fixture.playlist.entries().at(2).id();

    fixture.autoAdvance.acceptPlaybackState(40, false);
    fixture.autoAdvance.acceptPlaybackState(40, true);
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    // EOF already claimed generation 40 and queued B. A user Next arriving in
    // the same command-queue window must not advance the Playlist to C.
    QVERIFY(!fixture.controller.nextEntry());
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    fixture.autoAdvance.acceptPlaybackState(41, false);
    QVERIFY(fixture.controller.nextEntry());
    QCOMPARE(fixture.submissions, 2);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/c.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), thirdId.value());
}

void PlaylistAutoAdvanceTest::rapidManualNextAllowsOnlyOneAdvancePerObservedGeneration()
{
    Fixture fixture;
    openFour(fixture);
    const auto secondId = fixture.playlist.entries().at(1).id();
    const auto thirdId = fixture.playlist.entries().at(2).id();

    fixture.autoAdvance.acceptPlaybackState(50, false);
    QVERIFY(fixture.controller.nextEntry());
    for (int index = 0; index < 9; ++index) {
        QVERIFY(!fixture.controller.nextEntry());
    }

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    fixture.autoAdvance.acceptPlaybackState(51, false);
    QVERIFY(fixture.controller.nextEntry());
    QCOMPARE(fixture.submissions, 2);
    QCOMPARE(fixture.playlist.currentId()->value(), thirdId.value());
}

void PlaylistAutoAdvanceTest::acceptedStopSuppressesLateNaturalEnd()
{
    Fixture fixture;
    openTwo(fixture);
    const auto currentId = fixture.playlist.currentId();
    QVERIFY(currentId.has_value());

    fixture.autoAdvance.acceptPlaybackState(60, false);
    fixture.advanceArbiter.suppressObservedGeneration();
    fixture.autoAdvance.acceptPlaybackState(60, true);

    QCOMPARE(fixture.submissions, 0);
    QCOMPARE(fixture.playlist.currentId()->value(), currentId->value());
}

void PlaylistAutoAdvanceTest::rejectedManualNextReleasesClaimForNaturalEnd()
{
    Fixture fixture;
    openTwo(fixture);
    const auto secondId = fixture.playlist.entries().at(1).id();
    fixture.rejectedLocation = QStringLiteral("C:/media/b.mp4");

    fixture.autoAdvance.acceptPlaybackState(65, false);
    QVERIFY(!fixture.controller.nextEntry());
    QCOMPARE(fixture.submissions, 1);

    fixture.rejectedLocation.clear();
    fixture.autoAdvance.acceptPlaybackState(65, true);
    QCOMPARE(fixture.submissions, 2);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());
}

void PlaylistAutoAdvanceTest::repeatOneReloadsCurrent()
{
    Fixture fixture;
    openTwo(fixture);
    const auto currentId = fixture.playlist.currentId();
    QVERIFY(currentId.has_value());
    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::One);

    fixture.autoAdvance.acceptPlaybackState(70, true);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/a.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), currentId->value());
}

void PlaylistAutoAdvanceTest::repeatAllWrapsTail()
{
    Fixture fixture;
    openTwo(fixture);
    const auto firstId = fixture.playlist.entries().at(0).id();
    const auto secondId = fixture.playlist.entries().at(1).id();
    QVERIFY(fixture.controller.selectEntry(secondId.value()));
    fixture.submissions = 0;
    fixture.submittedLocation.clear();
    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::All);

    fixture.autoAdvance.acceptPlaybackState(80, true);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/a.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), firstId.value());
}

void PlaylistAutoAdvanceTest::shuffleAdvancesThroughControllerWithoutImmediateReplay()
{
    Fixture fixture;
    openTwo(fixture);
    const auto firstId = fixture.playlist.currentId();
    const auto secondId = fixture.playlist.entries().at(1).id();
    QVERIFY(firstId.has_value());
    fixture.playlist.setShuffleEnabled(true);

    fixture.autoAdvance.acceptPlaybackState(90, true);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());
    QVERIFY(fixture.playlist.currentId()->value() != firstId->value());

    fixture.autoAdvance.acceptPlaybackState(91, true);
    QCOMPARE(fixture.submissions, 1);
}

void PlaylistAutoAdvanceTest::failureSkipsNextAndDoesNotRetryOrWrap()
{
    Fixture fixture;
    openTwo(fixture);
    const auto secondId = fixture.playlist.entries().at(1).id();
    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::One);

    fixture.autoAdvance.acceptPlaybackFailure(100);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    fixture.autoAdvance.acceptPlaybackFailure(100);
    fixture.autoAdvance.acceptPlaybackState(100, true);
    QCOMPARE(fixture.submissions, 1);

    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::All);
    fixture.autoAdvance.acceptPlaybackState(101, false);
    fixture.autoAdvance.acceptPlaybackFailure(101);
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());
}

void PlaylistAutoAdvanceTest::shuffleFailureDoesNotRestartRepeatAllCycle()
{
    Fixture fixture;
    openTwo(fixture);
    const auto secondId = fixture.playlist.entries().at(1).id();
    fixture.playlist.setShuffleEnabled(true);
    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::All);

    fixture.autoAdvance.acceptPlaybackFailure(110);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    fixture.autoAdvance.acceptPlaybackState(111, false);
    fixture.autoAdvance.acceptPlaybackFailure(111);
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());
}

void PlaylistAutoAdvanceTest::rejectedAdvanceRestoresCurrentAndDoesNotRetrySameGeneration()
{
    Fixture fixture;
    openTwo(fixture);
    const auto firstId = fixture.playlist.currentId();
    QVERIFY(firstId.has_value());
    fixture.rejectedLocation = QStringLiteral("C:/media/b.mp4");

    fixture.autoAdvance.acceptPlaybackState(120, true);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.playlist.currentId()->value(), firstId->value());

    fixture.autoAdvance.acceptPlaybackState(120, false);
    fixture.autoAdvance.acceptPlaybackState(120, true);
    QCOMPARE(fixture.submissions, 1);
}

void PlaylistAutoAdvanceTest::newGenerationCanAdvanceAfterPreviousEnd()
{
    Fixture fixture;
    openTwo(fixture);
    const auto firstId = fixture.playlist.entries().at(0).id();

    fixture.autoAdvance.acceptPlaybackState(130, true);
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));

    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::All);
    fixture.autoAdvance.acceptPlaybackState(131, false);
    fixture.autoAdvance.acceptPlaybackState(131, true);

    QCOMPARE(fixture.submissions, 2);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/a.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), firstId.value());
}

} // namespace player::playlist::application

QTEST_GUILESS_MAIN(player::playlist::application::PlaylistAutoAdvanceTest)
#include "playlist_auto_advance_test.moc"
