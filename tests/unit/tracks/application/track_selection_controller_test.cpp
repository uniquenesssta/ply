#include "tracks/application/external_subtitle_loader.h"
#include "tracks/application/track_selection_controller.h"

#include "playback/domain/state/playback_snapshot.h"

#include <QFile>
#include <QMetaType>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

namespace player::tracks::application {
namespace {

using namespace player::playback::domain;

PlaybackSnapshot readySnapshot(const QList<TrackDescriptor>& tracks)
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{4};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.tracks.tracks = tracks;
    return PlaybackSnapshot{std::move(state)};
}

TrackDescriptor subtitleTrack(
    qint64 id,
    bool selected,
    bool external,
    QString externalFilename)
{
    TrackDescriptor track;
    track.id = id;
    track.kind = TrackKind::Subtitle;
    track.selected = selected;
    track.external = external;
    if (!externalFilename.isEmpty()) {
        track.externalFilename = std::move(externalFilename);
    }
    return track;
}

} // namespace

class TrackSelectionControllerTest final : public QObject
{
    Q_OBJECT

private slots:
    void rejectsSelectionWithoutCapability();
    void emitsSelectionIntent();
    void emitsDisableIntentWithNonPositiveId();
    void capabilityUpdatesEmitSignalOnlyOnChange();
};

void TrackSelectionControllerTest::rejectsSelectionWithoutCapability()
{
    TrackSelectionController controller;
    QVERIFY(!controller.canSelectAudio());
    QVERIFY(!controller.canSelectSubtitle());
    QVERIFY(!controller.canSelectVideo());

    QSignalSpy selectionSpy(&controller, &TrackSelectionController::trackSelectionRequested);
    QVERIFY(!controller.requestSelectAudioTrack(2));
    QVERIFY(!controller.requestSelectSubtitleTrack(5));
    QVERIFY(!controller.requestSelectVideoTrack(1));
    QCOMPARE(selectionSpy.count(), 0);
}

void TrackSelectionControllerTest::emitsSelectionIntent()
{
    TrackSelectionController controller;
    controller.acceptCapabilities(true, true, true);

    QSignalSpy selectionSpy(&controller, &TrackSelectionController::trackSelectionRequested);

    QVERIFY(controller.requestSelectAudioTrack(2));
    QCOMPARE(selectionSpy.count(), 1);
    QCOMPARE(selectionSpy.first().at(0).toInt(), static_cast<int>(TrackKind::Audio));
    QCOMPARE(selectionSpy.first().at(1).toLongLong(), qint64{2});

    QVERIFY(controller.requestSelectSubtitleTrack(5));
    QCOMPARE(selectionSpy.count(), 2);
    QCOMPARE(selectionSpy.at(1).at(0).toInt(), static_cast<int>(TrackKind::Subtitle));
    QCOMPARE(selectionSpy.at(1).at(1).toLongLong(), qint64{5});
}

void TrackSelectionControllerTest::emitsDisableIntentWithNonPositiveId()
{
    TrackSelectionController controller;
    controller.acceptCapabilities(true, true, true);

    QSignalSpy selectionSpy(&controller, &TrackSelectionController::trackSelectionRequested);

    QVERIFY(controller.requestSelectSubtitleTrack(0));
    QCOMPARE(selectionSpy.count(), 1);
    QCOMPARE(selectionSpy.first().at(1).toLongLong(), qint64{0});

    QVERIFY(controller.requestSelectAudioTrack(-1));
    QCOMPARE(selectionSpy.count(), 2);
    QCOMPARE(selectionSpy.at(1).at(1).toLongLong(), qint64{-1});
}

void TrackSelectionControllerTest::capabilityUpdatesEmitSignalOnlyOnChange()
{
    TrackSelectionController controller;
    QSignalSpy capabilitySpy(&controller, &TrackSelectionController::capabilityChanged);

    controller.acceptCapabilities(true, true, false);
    QCOMPARE(capabilitySpy.count(), 1);
    QVERIFY(controller.canSelectAudio());
    QVERIFY(controller.canSelectSubtitle());
    QVERIFY(!controller.canSelectVideo());

    controller.acceptCapabilities(true, true, false);
    QCOMPARE(capabilitySpy.count(), 1);

    controller.acceptCapabilities(true, false, false);
    QCOMPARE(capabilitySpy.count(), 2);
    QVERIFY(!controller.canSelectSubtitle());
}

class ExternalSubtitleLoaderTest final : public QObject
{
    Q_OBJECT

private slots:
    void rejectsWithoutActiveMedia();
    void rejectsEmptyMissingAndUnsupportedPaths();
    void loadsValidSubtitleAndRejectsRepeatedPath();
    void rejectsPathAlreadyPresentInTrackList();
    void emitsStateChangeOnMediaSwitch();
};

void ExternalSubtitleLoaderTest::rejectsWithoutActiveMedia()
{
    ExternalSubtitleLoader loader;
    QVERIFY(!loader.canLoad());

    QSignalSpy loadSpy(&loader, &ExternalSubtitleLoader::externalSubtitleLoadRequested);
    QVERIFY(!loader.requestLoad(QStringLiteral("C:/subs/x.srt")));
    QCOMPARE(loadSpy.count(), 0);
    QVERIFY(!loader.lastRejectionReason().isEmpty());
}

void ExternalSubtitleLoaderTest::rejectsEmptyMissingAndUnsupportedPaths()
{
    ExternalSubtitleLoader loader;
    loader.acceptSnapshot(readySnapshot({}));
    QVERIFY(loader.canLoad());

    QVERIFY(!loader.requestLoad(QString{}));
    QVERIFY(!loader.lastRejectionReason().isEmpty());

    QVERIFY(!loader.requestLoad(QStringLiteral("C:/missing/track.srt")));
    QVERIFY(!loader.lastRejectionReason().isEmpty());

    QVERIFY(!loader.requestLoad(QStringLiteral("C:/subs/track.txt")));
    QVERIFY(!loader.lastRejectionReason().isEmpty());
}

void ExternalSubtitleLoaderTest::loadsValidSubtitleAndRejectsRepeatedPath()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString subtitlePath = temporaryDirectory.filePath(QStringLiteral("movie.srt"));
    QFile subtitleFile(subtitlePath);
    QVERIFY(subtitleFile.open(QIODevice::WriteOnly | QIODevice::Text));
    subtitleFile.write("1\n00:00:01,000 --> 00:00:02,000\nHello\n");
    subtitleFile.close();

    ExternalSubtitleLoader loader;
    loader.acceptSnapshot(readySnapshot({}));

    QSignalSpy loadSpy(&loader, &ExternalSubtitleLoader::externalSubtitleLoadRequested);
    QVERIFY(loader.requestLoad(subtitlePath));
    QCOMPARE(loadSpy.count(), 1);
    QVERIFY(loader.lastRejectionReason().isEmpty());

    // The loaded path now appears in the authoritative track list; the
    // second request must be rejected as a duplicate.
    loader.acceptSnapshot(readySnapshot({
        subtitleTrack(3, true, true, subtitlePath),
    }));
    QVERIFY(!loader.requestLoad(subtitlePath));
    QCOMPARE(loadSpy.count(), 1);
    QCOMPARE(loader.lastRejectionReason(), QStringLiteral("This subtitle is already loaded."));
}

void ExternalSubtitleLoaderTest::rejectsPathAlreadyPresentInTrackList()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString subtitlePath = temporaryDirectory.filePath(QStringLiteral("already.srt"));
    QFile subtitleFile(subtitlePath);
    QVERIFY(subtitleFile.open(QIODevice::WriteOnly | QIODevice::Text));
    subtitleFile.write("1\n00:00:01,000 --> 00:00:02,000\nHello\n");
    subtitleFile.close();

    ExternalSubtitleLoader loader;
    loader.acceptSnapshot(readySnapshot({
        subtitleTrack(4, true, true, subtitlePath),
    }));

    QSignalSpy loadSpy(&loader, &ExternalSubtitleLoader::externalSubtitleLoadRequested);
    QVERIFY(!loader.requestLoad(subtitlePath));
    QCOMPARE(loadSpy.count(), 0);
    QCOMPARE(loader.lastRejectionReason(), QStringLiteral("This subtitle is already loaded."));
}

void ExternalSubtitleLoaderTest::emitsStateChangeOnMediaSwitch()
{
    ExternalSubtitleLoader loader;
    QSignalSpy stateSpy(&loader, &ExternalSubtitleLoader::stateChanged);

    loader.acceptSnapshot(readySnapshot({subtitleTrack(1, false, true, QStringLiteral("C:/subs/a.srt"))}));
    QVERIFY(loader.canLoad());
    QCOMPARE(stateSpy.count(), 1);

    loader.acceptSnapshot(readySnapshot({subtitleTrack(1, false, true, QStringLiteral("C:/subs/a.srt"))}));
    QCOMPARE(stateSpy.count(), 1);

    loader.acceptSnapshot(PlaybackSnapshot::stopped(MediaGeneration{5}));
    QVERIFY(!loader.canLoad());
    QCOMPARE(stateSpy.count(), 2);
}

} // namespace player::tracks::application

QTEST_GUILESS_MAIN(player::tracks::application::TrackSelectionControllerTest)
#include "track_selection_controller_test.moc"
