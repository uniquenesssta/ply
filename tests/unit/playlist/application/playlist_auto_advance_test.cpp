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
    void rejectedAdvanceRestoresCurrentAndDoesNotRetrySameGeneration();
    void sameGenerationCanEndAgainAfterLeavingEndedState();
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

    fixture.autoAdvance.acceptPlaybackState(13, true);
    QCOMPARE(fixture.submissions, 1);
}

void PlaylistAutoAdvanceTest::sameGenerationCanEndAgainAfterLeavingEndedState()
{
    Fixture fixture;
    openTwo(fixture);
    fixture.rejectedLocation = QStringLiteral("C:/media/b.mp4");

    fixture.autoAdvance.acceptPlaybackState(21, true);
    QCOMPARE(fixture.submissions, 1);

    fixture.autoAdvance.acceptPlaybackState(21, false);
    fixture.rejectedLocation.clear();
    fixture.autoAdvance.acceptPlaybackState(21, true);

    QCOMPARE(fixture.submissions, 2);
    QCOMPARE(fixture.submittedLocation, QStringLiteral("C:/media/b.mp4"));
}

} // namespace player::playlist::application

QTEST_GUILESS_MAIN(player::playlist::application::PlaylistAutoAdvanceTest)
#include "playlist_auto_advance_test.moc"
