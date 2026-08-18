#include "media/domain/media_source.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/application/playlist_mutation.h"
#include "playlist/domain/playlist.h"

#include <QSignalSpy>
#include <QtTest>

namespace player::playlist::application {
namespace {

media::domain::MediaSource localSource(const QString& path)
{
    return media::domain::MediaSource::localFile(path);
}

} // namespace

class PlaylistMutationSemanticsTest final : public QObject
{
    Q_OBJECT

private slots:
    void appendAndInsertPreserveCurrentWithoutPlaybackSubmission();
    void replaceQueueCommitsOnlyAfterAcceptedLoad();
    void rejectedQueueReplacementPreservesStateAndIdAllocator();
    void clearStopsBeforeMutationAndRejectedStopIsAtomic();
    void manualNextPreviousRespectOrderRepeatAndLoadRejection();
    void shuffleManualNextCommitsOnlyAfterAcceptedLoad();
    void modeChangesPreserveOrderAndCurrent();
    void selectingCurrentIsIdempotentWithoutReload();
};

void PlaylistMutationSemanticsTest::appendAndInsertPreserveCurrentWithoutPlaybackSubmission()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    int loadSubmissions = 0;
    PlaylistController controller(
        playlist,
        mutation,
        [&](const media::domain::MediaSource&) {
            ++loadSubmissions;
            return true;
        });

    QVERIFY(controller.openSource(localSource(QStringLiteral("C:/media/a.mp4"))));
    const auto originalCurrent = playlist.currentId();
    QVERIFY(originalCurrent.has_value());
    QCOMPARE(loadSubmissions, 1);

    QVERIFY(controller.appendSources({
        localSource(QStringLiteral("C:/media/b.mp4")),
        localSource(QStringLiteral("C:/media/c.mp4")),
    }));
    QCOMPARE(loadSubmissions, 1);
    QCOMPARE(playlist.size(), std::size_t{3});
    QCOMPARE(playlist.currentId()->value(), originalCurrent->value());

    QVERIFY(controller.insertSource(
        localSource(QStringLiteral("C:/media/d.mp4")),
        1));
    QCOMPARE(loadSubmissions, 1);
    QCOMPARE(playlist.size(), std::size_t{4});
    QCOMPARE(playlist.entries().at(1).source().location(), QStringLiteral("C:/media/d.mp4"));
    QCOMPARE(playlist.currentId()->value(), originalCurrent->value());
    QCOMPARE(playlist.entries().at(1).id().value(), quint64{4});

    QVERIFY(!controller.insertSource(
        localSource(QStringLiteral("C:/media/out-of-range.mp4")),
        99));
    QCOMPARE(playlist.size(), std::size_t{4});
    QCOMPARE(playlist.currentId()->value(), originalCurrent->value());
}

void PlaylistMutationSemanticsTest::replaceQueueCommitsOnlyAfterAcceptedLoad()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    bool replacing = false;
    bool submissionSawOldState = false;
    QString submittedLocation;
    domain::PlaylistEntryId oldCurrent;

    PlaylistController controller(
        playlist,
        mutation,
        [&](const media::domain::MediaSource& source) {
            submittedLocation = source.location();
            if (replacing) {
                submissionSawOldState = playlist.size() == std::size_t{2}
                    && playlist.currentId().has_value()
                    && *playlist.currentId() == oldCurrent
                    && playlist.entries().at(0).source().location()
                        == QStringLiteral("C:/media/a.mp4");
            }
            return true;
        });

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
    }));
    oldCurrent = *playlist.currentId();
    QVERIFY(controller.setRepeatMode(domain::PlaylistRepeatMode::All));
    QVERIFY(controller.setShuffleEnabled(true));

    replacing = true;
    submittedLocation.clear();
    QVERIFY(controller.replaceSources({
        localSource(QStringLiteral("C:/media/c.mp4")),
        localSource(QStringLiteral("C:/media/d.mp4")),
    }));

    QVERIFY(submissionSawOldState);
    QCOMPARE(submittedLocation, QStringLiteral("C:/media/c.mp4"));
    QCOMPARE(playlist.size(), std::size_t{2});
    QCOMPARE(playlist.entries().at(0).source().location(), QStringLiteral("C:/media/c.mp4"));
    QCOMPARE(playlist.entries().at(1).source().location(), QStringLiteral("C:/media/d.mp4"));
    QCOMPARE(playlist.entries().at(0).id().value(), quint64{3});
    QCOMPARE(playlist.entries().at(1).id().value(), quint64{4});
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), quint64{3});
    QCOMPARE(
        static_cast<int>(playlist.repeatMode()),
        static_cast<int>(domain::PlaylistRepeatMode::All));
    QVERIFY(playlist.shuffleEnabled());
    QVERIFY(!playlist.snapshot().shuffleCycleInitialized());
}

void PlaylistMutationSemanticsTest::rejectedQueueReplacementPreservesStateAndIdAllocator()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    bool rejectReplacement = false;
    PlaylistController controller(
        playlist,
        mutation,
        [&](const media::domain::MediaSource& source) {
            return !(rejectReplacement
                && source.location() == QStringLiteral("C:/media/c.mp4"));
        });

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
    }));
    const domain::PlaylistSnapshot before = controller.snapshot();

    rejectReplacement = true;
    QVERIFY(!controller.replaceSources({
        localSource(QStringLiteral("C:/media/c.mp4")),
        localSource(QStringLiteral("C:/media/d.mp4")),
    }));

    const domain::PlaylistSnapshot after = controller.snapshot();
    QCOMPARE(after.size(), before.size());
    QCOMPARE(after.entries().at(0).id().value(), before.entries().at(0).id().value());
    QCOMPARE(after.entries().at(1).id().value(), before.entries().at(1).id().value());
    QVERIFY(after.currentId().has_value());
    QCOMPARE(after.currentId()->value(), before.currentId()->value());

    QVERIFY(controller.appendSources({
        localSource(QStringLiteral("C:/media/e.mp4")),
    }));
    QCOMPARE(playlist.entries().back().id().value(), quint64{3});
}

void PlaylistMutationSemanticsTest::clearStopsBeforeMutationAndRejectedStopIsAtomic()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    bool allowStop = false;
    bool stopSawOriginalState = false;
    int stopSubmissions = 0;
    PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource&) { return true; },
        [&]() {
            ++stopSubmissions;
            stopSawOriginalState = playlist.size() == std::size_t{2}
                && playlist.currentId().has_value();
            return allowStop;
        },
        nullptr);

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
    }));
    const auto current = playlist.currentId();
    QVERIFY(current.has_value());
    QVERIFY(controller.setRepeatMode(domain::PlaylistRepeatMode::All));
    QVERIFY(controller.setShuffleEnabled(true));

    QVERIFY(!controller.clearQueue());
    QCOMPARE(stopSubmissions, 1);
    QVERIFY(stopSawOriginalState);
    QCOMPARE(playlist.size(), std::size_t{2});
    QCOMPARE(playlist.currentId()->value(), current->value());

    allowStop = true;
    QVERIFY(controller.clearQueue());
    QCOMPARE(stopSubmissions, 2);
    QVERIFY(playlist.empty());
    QVERIFY(!playlist.currentId().has_value());
    QCOMPARE(
        static_cast<int>(playlist.repeatMode()),
        static_cast<int>(domain::PlaylistRepeatMode::All));
    QVERIFY(playlist.shuffleEnabled());

    QVERIFY(controller.clearQueue());
    QCOMPARE(stopSubmissions, 2);
}

void PlaylistMutationSemanticsTest::manualNextPreviousRespectOrderRepeatAndLoadRejection()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    QString rejectedLocation;
    QString lastSubmitted;
    PlaylistController controller(
        playlist,
        mutation,
        [&](const media::domain::MediaSource& source) {
            lastSubmitted = source.location();
            return source.location() != rejectedLocation;
        });

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
        localSource(QStringLiteral("C:/media/c.mp4")),
    }));
    const auto first = playlist.entries().at(0).id();
    const auto second = playlist.entries().at(1).id();
    const auto third = playlist.entries().at(2).id();

    QVERIFY(controller.nextEntry());
    QCOMPARE(lastSubmitted, QStringLiteral("C:/media/b.mp4"));
    QCOMPARE(playlist.currentId()->value(), second.value());

    QVERIFY(controller.previousEntry());
    QCOMPARE(lastSubmitted, QStringLiteral("C:/media/a.mp4"));
    QCOMPARE(playlist.currentId()->value(), first.value());
    QVERIFY(!controller.previousEntry());
    QCOMPARE(playlist.currentId()->value(), first.value());

    QVERIFY(controller.setRepeatMode(domain::PlaylistRepeatMode::All));
    QVERIFY(controller.previousEntry());
    QCOMPARE(lastSubmitted, QStringLiteral("C:/media/c.mp4"));
    QCOMPARE(playlist.currentId()->value(), third.value());

    QVERIFY(controller.selectEntry(second.value()));
    QVERIFY(controller.setRepeatMode(domain::PlaylistRepeatMode::One));
    QVERIFY(controller.nextEntry());
    QCOMPARE(playlist.currentId()->value(), third.value());

    QVERIFY(controller.selectEntry(second.value()));
    rejectedLocation = QStringLiteral("C:/media/c.mp4");
    QVERIFY(!controller.nextEntry());
    QCOMPARE(lastSubmitted, QStringLiteral("C:/media/c.mp4"));
    QCOMPARE(playlist.currentId()->value(), second.value());
}

void PlaylistMutationSemanticsTest::shuffleManualNextCommitsOnlyAfterAcceptedLoad()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    bool rejectNavigation = false;
    PlaylistController controller(
        playlist,
        mutation,
        [&](const media::domain::MediaSource&) {
            return !rejectNavigation;
        });

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
        localSource(QStringLiteral("C:/media/c.mp4")),
    }));
    const auto originalCurrent = playlist.currentId();
    QVERIFY(originalCurrent.has_value());
    QVERIFY(controller.setShuffleEnabled(true));
    QVERIFY(!playlist.snapshot().shuffleCycleInitialized());

    rejectNavigation = true;
    QVERIFY(!controller.nextEntry());
    QCOMPARE(playlist.currentId()->value(), originalCurrent->value());
    QVERIFY(!playlist.snapshot().shuffleCycleInitialized());
    QVERIFY(playlist.snapshot().shuffleRemainingEntryIds().empty());

    rejectNavigation = false;
    QVERIFY(controller.nextEntry());
    QVERIFY(playlist.currentId().has_value());
    QVERIFY(playlist.currentId()->value() != originalCurrent->value());
    const domain::PlaylistSnapshot after = playlist.snapshot();
    QVERIFY(after.shuffleCycleInitialized());
    QCOMPARE(after.shuffleRemainingEntryIds().size(), std::size_t{1});

    QVERIFY(!controller.previousEntry());
}

void PlaylistMutationSemanticsTest::modeChangesPreserveOrderAndCurrent()
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
    const auto current = playlist.currentId();
    QVERIFY(current.has_value());
    const auto first = playlist.entries().at(0).id();
    const auto second = playlist.entries().at(1).id();
    const auto third = playlist.entries().at(2).id();

    QSignalSpy changedSpy(&controller, &PlaylistController::playlistChanged);
    QVERIFY(controller.setRepeatMode(domain::PlaylistRepeatMode::All));
    QVERIFY(controller.setRepeatMode(domain::PlaylistRepeatMode::All));
    QVERIFY(controller.setShuffleEnabled(true));
    QVERIFY(controller.setShuffleEnabled(true));
    QVERIFY(controller.setShuffleEnabled(false));

    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(playlist.entries().at(0).id().value(), first.value());
    QCOMPARE(playlist.entries().at(1).id().value(), second.value());
    QCOMPARE(playlist.entries().at(2).id().value(), third.value());
    QCOMPARE(playlist.currentId()->value(), current->value());
    QCOMPARE(
        static_cast<int>(playlist.repeatMode()),
        static_cast<int>(domain::PlaylistRepeatMode::All));
    QVERIFY(!playlist.shuffleEnabled());
}

void PlaylistMutationSemanticsTest::selectingCurrentIsIdempotentWithoutReload()
{
    domain::Playlist playlist;
    PlaylistMutation mutation(playlist);
    int loadSubmissions = 0;
    PlaylistController controller(
        playlist,
        mutation,
        [&](const media::domain::MediaSource&) {
            ++loadSubmissions;
            return true;
        });

    QVERIFY(controller.openSource(localSource(QStringLiteral("C:/media/a.mp4"))));
    const auto current = playlist.currentId();
    QVERIFY(current.has_value());
    QCOMPARE(loadSubmissions, 1);

    QSignalSpy changedSpy(&controller, &PlaylistController::playlistChanged);
    QVERIFY(controller.selectEntry(current->value()));
    QCOMPARE(loadSubmissions, 1);
    QCOMPARE(changedSpy.count(), 0);
    QCOMPARE(playlist.currentId()->value(), current->value());
}

} // namespace player::playlist::application

QTEST_GUILESS_MAIN(player::playlist::application::PlaylistMutationSemanticsTest)
#include "playlist_mutation_semantics_test.moc"
