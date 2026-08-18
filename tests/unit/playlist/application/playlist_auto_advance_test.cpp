#include "media/domain/media_source.h"
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
    int submissions = 0;
    QString submittedLocation;
    QString rejectedLocation;
    PlaylistController controller{
        playlist,
        mutation,
        [this](const media::domain::MediaSource& source) {
            ++submissions;
            submittedLocation = source.location();
            return source.location() != rejectedLocation;
        }};
    PlaylistAutoAdvance autoAdvance{playlist, controller};
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

} // namespace

class PlaylistAutoAdvanceTest final : public QObject
{
    Q_OBJECT

private slots:
    void naturalEndAdvancesExactlyOnceWhileStopIsIgnored();
    void repeatOneReloadsCurrent();
    void repeatAllWrapsTail();
    void shuffleAdvancesThroughControllerWithoutImmediateReplay();
    void failureSkipsNextAndDoesNotRetryOrWrap();
    void shuffleFailureDoesNotRestartRepeatAllCycle();
    void rejectedAdvanceRestoresCurrentAndDoesNotRetrySameGeneration();
    void newGenerationCanAdvanceAfterPreviousEnd();
};

void PlaylistAutoAdvanceTest::naturalEndAdvancesExactlyOnceWhileStopIsIgnored()
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

void PlaylistAutoAdvanceTest::repeatOneReloadsCurrent()
{
    Fixture fixture;
    openTwo(fixture);
    const auto currentId = fixture.playlist.currentId();
    QVERIFY(currentId.has_value());
    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::One);

    fixture.autoAdvance.acceptPlaybackState(5, true);

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

    fixture.autoAdvance.acceptPlaybackState(8, true);

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

    fixture.autoAdvance.acceptPlaybackState(11, true);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());
    QVERIFY(fixture.playlist.currentId()->value() != firstId->value());

    fixture.autoAdvance.acceptPlaybackState(12, true);
    QCOMPARE(fixture.submissions, 1);
}

void PlaylistAutoAdvanceTest::failureSkipsNextAndDoesNotRetryOrWrap()
{
    Fixture fixture;
    openTwo(fixture);
    const auto secondId = fixture.playlist.entries().at(1).id();
    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::One);

    fixture.autoAdvance.acceptPlaybackFailure(15);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    fixture.autoAdvance.acceptPlaybackFailure(15);
    fixture.autoAdvance.acceptPlaybackState(15, true);
    QCOMPARE(fixture.submissions, 1);

    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::All);
    fixture.autoAdvance.acceptPlaybackState(16, false);
    fixture.autoAdvance.acceptPlaybackFailure(16);
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

    fixture.autoAdvance.acceptPlaybackFailure(17);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), secondId.value());

    fixture.autoAdvance.acceptPlaybackState(18, false);
    fixture.autoAdvance.acceptPlaybackFailure(18);
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

    fixture.autoAdvance.acceptPlaybackState(13, true);

    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.playlist.currentId()->value(), firstId->value());

    fixture.autoAdvance.acceptPlaybackState(13, false);
    fixture.autoAdvance.acceptPlaybackState(13, true);
    QCOMPARE(fixture.submissions, 1);
}

void PlaylistAutoAdvanceTest::newGenerationCanAdvanceAfterPreviousEnd()
{
    Fixture fixture;
    openTwo(fixture);
    const auto firstId = fixture.playlist.entries().at(0).id();

    fixture.autoAdvance.acceptPlaybackState(21, true);
    QCOMPARE(fixture.submissions, 1);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));

    fixture.playlist.setRepeatMode(domain::PlaylistRepeatMode::All);
    fixture.autoAdvance.acceptPlaybackState(22, false);
    fixture.autoAdvance.acceptPlaybackState(22, true);

    QCOMPARE(fixture.submissions, 2);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/a.mp4"));
    QCOMPARE(fixture.playlist.currentId()->value(), firstId.value());
}

} // namespace player::playlist::application

QTEST_GUILESS_MAIN(player::playlist::application::PlaylistAutoAdvanceTest)
#include "playlist_auto_advance_test.moc"
