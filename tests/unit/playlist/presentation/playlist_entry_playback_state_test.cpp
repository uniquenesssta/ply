#include "media/domain/media_source.h"
#include "playback/domain/state/media_generation.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/application/playlist_mutation.h"
#include "playlist/domain/playlist.h"
#include "playlist/presentation/playlist_entry_playback_state.h"

#include <QSignalSpy>
#include <QtTest>

#include <utility>

namespace player::playlist::presentation {
namespace {

media::domain::MediaSource localSource(const QString& path)
{
    return media::domain::MediaSource::localFile(path);
}

playback::domain::PlaybackSnapshot playbackSnapshot(
    quint64 generation,
    playback::domain::PlaybackLifecycleState lifecycle)
{
    playback::domain::PlaybackSnapshotState state;
    state.generation = playback::domain::MediaGeneration{generation};
    state.lifecycle = lifecycle;
    return playback::domain::PlaybackSnapshot{std::move(state)};
}

} // namespace

class PlaylistEntryPlaybackStateTest final : public QObject
{
    Q_OBJECT

private slots:
    void tracksPendingUnavailableAndRetryIndependentlyFromCurrent();
    void clearsPendingOnCurrentChangeAndPrunesRemovedEntries();
};

void PlaylistEntryPlaybackStateTest::tracksPendingUnavailableAndRetryIndependentlyFromCurrent()
{
    domain::Playlist playlist;
    application::PlaylistMutation mutation(playlist);
    application::PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource&) { return true; });
    PlaylistEntryPlaybackState state(controller);
    QSignalSpy stateSpy(&state, &PlaylistEntryPlaybackState::entryStatesChanged);

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
    }));
    const domain::PlaylistEntryId firstId = playlist.entries().at(0).id();
    const domain::PlaylistEntryId secondId = playlist.entries().at(1).id();

    state.acceptPlaybackSnapshot(playbackSnapshot(
        1,
        playback::domain::PlaybackLifecycleState::Opening));
    QVERIFY(state.isPendingLoading(firstId));
    QVERIFY(!state.isUnavailable(firstId));

    state.acceptPlaybackSnapshot(playbackSnapshot(
        1,
        playback::domain::PlaybackLifecycleState::Failed));
    QVERIFY(!state.isPendingLoading(firstId));
    QVERIFY(state.isUnavailable(firstId));
    QVERIFY(playlist.currentId().has_value());
    QCOMPARE(playlist.currentId()->value(), firstId.value());

    QVERIFY(controller.selectEntry(secondId.value()));
    QVERIFY(state.isUnavailable(firstId));
    QVERIFY(!state.isPendingLoading(firstId));

    state.acceptPlaybackSnapshot(playbackSnapshot(
        2,
        playback::domain::PlaybackLifecycleState::Opening));
    QVERIFY(state.isPendingLoading(secondId));
    QVERIFY(!state.isUnavailable(secondId));

    state.acceptPlaybackSnapshot(playbackSnapshot(
        2,
        playback::domain::PlaybackLifecycleState::Ready));
    QVERIFY(!state.isPendingLoading(secondId));

    QVERIFY(controller.selectEntry(firstId.value()));
    state.acceptPlaybackSnapshot(playbackSnapshot(
        3,
        playback::domain::PlaybackLifecycleState::Opening));
    QVERIFY(state.isPendingLoading(firstId));
    QVERIFY(!state.isUnavailable(firstId));
    QVERIFY(stateSpy.count() >= 5);
}

void PlaylistEntryPlaybackStateTest::clearsPendingOnCurrentChangeAndPrunesRemovedEntries()
{
    domain::Playlist playlist;
    application::PlaylistMutation mutation(playlist);
    application::PlaylistController controller(
        playlist,
        mutation,
        [](const media::domain::MediaSource&) { return true; });
    PlaylistEntryPlaybackState state(controller);

    QVERIFY(controller.openSources({
        localSource(QStringLiteral("C:/media/a.mp4")),
        localSource(QStringLiteral("C:/media/b.mp4")),
    }));
    const domain::PlaylistEntryId firstId = playlist.entries().at(0).id();
    const domain::PlaylistEntryId secondId = playlist.entries().at(1).id();

    state.acceptPlaybackSnapshot(playbackSnapshot(
        1,
        playback::domain::PlaybackLifecycleState::Opening));
    QVERIFY(state.isPendingLoading(firstId));

    QVERIFY(controller.selectEntry(secondId.value()));
    QVERIFY(!state.isPendingLoading(firstId));

    QVERIFY(controller.selectEntry(firstId.value()));
    state.acceptPlaybackSnapshot(playbackSnapshot(
        2,
        playback::domain::PlaybackLifecycleState::Failed));
    QVERIFY(state.isUnavailable(firstId));

    QVERIFY(controller.selectEntry(secondId.value()));
    QVERIFY(controller.removeEntry(firstId.value()));
    QVERIFY(!state.isUnavailable(firstId));
    QVERIFY(!state.isPendingLoading(firstId));
}

} // namespace player::playlist::presentation

QTEST_GUILESS_MAIN(player::playlist::presentation::PlaylistEntryPlaybackStateTest)
#include "playlist_entry_playback_state_test.moc"
