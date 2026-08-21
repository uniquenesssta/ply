#include "foundation/ids/request_id.h"
#include "playback/application/requests/request_tracker.h"
#include "playback/domain/commands/external_subtitle_command.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/models/track_descriptor.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"
#include "playback/infrastructure/mpv/commands/mpv_command_encoder.h"
#include "playback/infrastructure/mpv/commands/mpv_playback_command_mapper.h"
#include "tracks/application/external_subtitle_loader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QString>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

#include <optional>
#include <utility>

namespace player::tracks::application {
namespace {

QString createFile(const QTemporaryDir& directory, const QString& name, const QByteArray& contents)
{
    const QString path = directory.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    if (file.write(contents) != contents.size()) {
        return {};
    }
    file.close();
    return path;
}

QString readSource(const QString& relativePath)
{
    QFile file(QDir(QStringLiteral(PLAYER_SOURCE_DIR)).filePath(relativePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

player::playback::domain::PlaybackSnapshot readySnapshot(
    player::playback::domain::MediaGeneration generation,
    QList<player::playback::domain::TrackDescriptor> tracks = {})
{
    player::playback::domain::PlaybackSnapshotState state;
    state.generation = generation;
    state.lifecycle = player::playback::domain::PlaybackLifecycleState::Ready;
    state.tracks.tracks = std::move(tracks);
    return player::playback::domain::PlaybackSnapshot{std::move(state)};
}

} // namespace

class ExternalSubtitleFlowTest final : public QObject
{
    Q_OBJECT

private slots:
    void loaderAcceptsReadableSrtAndAss();
    void loaderRejectsInvalidLocalSources();
    void loaderReportsSubmissionFailure();
    void loaderDeduplicatesPendingAcceptedAndSnapshotPaths();
    void loaderCorrelatesFailuresMediaSwitchAndShutdown();
    void loaderHandlesFileRemovalAfterValidation();
    void commandFlowUsesGenerationScopedCachedSubAdd();
    void successfulReplyRefreshesExternalSubtitleTrackState();
    void qmlKeepsFilePickingOutsideTrackPopup();
};

void ExternalSubtitleFlowTest::loaderAcceptsReadableSrtAndAss()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString srtPath = createFile(
        directory,
        QStringLiteral("captions.srt"),
        QByteArrayLiteral("1\n00:00:00,000 --> 00:00:01,000\nHello\n"));
    const QString assPath = createFile(
        directory,
        QStringLiteral("captions.ASS"),
        QByteArrayLiteral("[Script Info]\nScriptType: v4.00+\n"));
    QVERIFY(!srtPath.isEmpty());
    QVERIFY(!assPath.isEmpty());

    QList<QString> submittedSources;
    quint64 nextRequestId = 1;
    ExternalSubtitleLoader loader(
        [&submittedSources, &nextRequestId](
            const player::playback::domain::AddExternalSubtitleCommand& command) {
            submittedSources.append(command.source);
            return player::ids::RequestId{nextRequestId++};
        });
    loader.acceptSnapshot(readySnapshot(player::playback::domain::MediaGeneration{7}));

    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(srtPath)));
    QVERIFY(loader.lastErrorKey().isEmpty());
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(assPath)));
    QVERIFY(loader.lastErrorKey().isEmpty());

    QCOMPARE(submittedSources.size(), 2);
    QCOMPARE(submittedSources.at(0), QFileInfo(srtPath).canonicalFilePath());
    QCOMPARE(submittedSources.at(1), QFileInfo(assPath).canonicalFilePath());
}

void ExternalSubtitleFlowTest::loaderRejectsInvalidLocalSources()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    int submissions = 0;
    ExternalSubtitleLoader loader(
        [&submissions](const player::playback::domain::AddExternalSubtitleCommand&) {
            ++submissions;
            return std::optional{player::ids::RequestId{static_cast<quint64>(submissions)}};
        });
    loader.acceptSnapshot(readySnapshot(player::playback::domain::MediaGeneration{7}));

    QVERIFY(!loader.loadLocalSubtitle(QUrl(QStringLiteral("https://example.invalid/captions.srt"))));
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("not-local-file"));

    QVERIFY(!loader.loadLocalSubtitle(
        QUrl::fromLocalFile(directory.filePath(QStringLiteral("missing.srt")))));
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("file-unavailable"));

    const QString unsupportedPath = createFile(
        directory,
        QStringLiteral("captions.txt"),
        QByteArrayLiteral("subtitle"));
    QVERIFY(!unsupportedPath.isEmpty());
    QVERIFY(!loader.loadLocalSubtitle(QUrl::fromLocalFile(unsupportedPath)));
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("unsupported-format"));

    QCOMPARE(submissions, 0);
}

void ExternalSubtitleFlowTest::loaderReportsSubmissionFailure()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString subtitlePath = createFile(
        directory,
        QStringLiteral("captions.srt"),
        QByteArrayLiteral("1\n00:00:00,000 --> 00:00:01,000\nHello\n"));
    QVERIFY(!subtitlePath.isEmpty());

    ExternalSubtitleLoader loader(
        [](const player::playback::domain::AddExternalSubtitleCommand&) {
            return std::optional<player::ids::RequestId>{};
        });

    QVERIFY(!loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("no-active-media"));

    loader.acceptSnapshot(readySnapshot(player::playback::domain::MediaGeneration{7}));

    QVERIFY(!loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("submission-failed"));
}

void ExternalSubtitleFlowTest::loaderDeduplicatesPendingAcceptedAndSnapshotPaths()
{
    using namespace player::playback::domain;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString subtitlePath = createFile(
        directory,
        QStringLiteral("captions.srt"),
        QByteArrayLiteral("1\n00:00:00,000 --> 00:00:01,000\nHello\n"));
    QVERIFY(!subtitlePath.isEmpty());

    QList<QString> submittedSources;
    quint64 nextRequestId = 10;
    ExternalSubtitleLoader loader(
        [&submittedSources, &nextRequestId](const AddExternalSubtitleCommand& command) {
            submittedSources.append(command.source);
            return std::optional{player::ids::RequestId{nextRequestId++}};
        });
    QSignalSpy loadingSpy(&loader, &ExternalSubtitleLoader::loadingChanged);

    const MediaGeneration generationA{41};
    loader.acceptSnapshot(readySnapshot(generationA));
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QVERIFY(loader.loading());
    QCOMPARE(submittedSources.size(), 1);

    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QVERIFY(loader.loading());
    QCOMPARE(submittedSources.size(), 1);

    loader.acceptRequestResult(player::ids::RequestId{10}, generationA, true, {});
    QVERIFY(!loader.loading());
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QCOMPARE(submittedSources.size(), 1);

    TrackDescriptor externalTrack;
    externalTrack.id = 7;
    externalTrack.kind = TrackKind::Subtitle;
    externalTrack.external = true;
    externalTrack.externalFilename = QFileInfo(subtitlePath).canonicalFilePath();
    loader.acceptSnapshot(readySnapshot(generationA, {externalTrack}));
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QCOMPARE(submittedSources.size(), 1);

    QVERIFY(QFile::remove(subtitlePath));
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QCOMPARE(submittedSources.size(), 1);
    QCOMPARE(
        createFile(
            directory,
            QStringLiteral("captions.srt"),
            QByteArrayLiteral("1\n00:00:00,000 --> 00:00:01,000\nHello again\n")),
        subtitlePath);

    loader.acceptSnapshot(readySnapshot(MediaGeneration{42}));
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QCOMPARE(submittedSources.size(), 2);
    QVERIFY(loader.loading());
    QCOMPARE(loadingSpy.count(), 3);
}

void ExternalSubtitleFlowTest::loaderCorrelatesFailuresMediaSwitchAndShutdown()
{
    using namespace player::playback::domain;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstPath = createFile(
        directory,
        QStringLiteral("first.srt"),
        QByteArrayLiteral("invalid subtitle payload"));
    const QString secondPath = createFile(
        directory,
        QStringLiteral("second.ass"),
        QByteArrayLiteral("[Script Info]\nScriptType: v4.00+\n"));
    QVERIFY(!firstPath.isEmpty());
    QVERIFY(!secondPath.isEmpty());

    quint64 nextRequestId = 51;
    int submissions = 0;
    ExternalSubtitleLoader loader(
        [&nextRequestId, &submissions](const AddExternalSubtitleCommand&) {
            ++submissions;
            return std::optional{player::ids::RequestId{nextRequestId++}};
        });

    const MediaGeneration generationA{8};
    loader.acceptSnapshot(readySnapshot(generationA));
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(firstPath)));
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(secondPath)));
    QVERIFY(loader.loading());
    QCOMPARE(submissions, 2);

    loader.acceptRequestResult(
        player::ids::RequestId{51},
        generationA,
        false,
        QStringLiteral("unsupported subtitle format"));
    QVERIFY(loader.loading());
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("backend-rejected"));

    loader.acceptRequestResult(player::ids::RequestId{52}, generationA, true, {});
    QVERIFY(!loader.loading());
    QVERIFY(loader.lastErrorKey().isEmpty());
    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(secondPath)));
    QCOMPARE(submissions, 2);

    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(firstPath)));
    QVERIFY(loader.loading());
    QCOMPARE(submissions, 3);
    loader.acceptSnapshot(readySnapshot(MediaGeneration{9}));
    QVERIFY(!loader.loading());
    loader.acceptRequestResult(
        player::ids::RequestId{53},
        generationA,
        false,
        QStringLiteral("late stale failure"));
    QVERIFY(loader.lastErrorKey().isEmpty());

    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(firstPath)));
    QVERIFY(loader.loading());
    loader.beginShutdown();
    QVERIFY(!loader.loading());
    QVERIFY(!loader.loadLocalSubtitle(QUrl::fromLocalFile(firstPath)));
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("shutting-down"));
}

void ExternalSubtitleFlowTest::loaderHandlesFileRemovalAfterValidation()
{
    using namespace player::playback::domain;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString subtitlePath = createFile(
        directory,
        QStringLiteral("removed-after-validation.srt"),
        QByteArrayLiteral("1\n00:00:00,000 --> 00:00:01,000\nHello\n"));
    QVERIFY(!subtitlePath.isEmpty());

    ExternalSubtitleLoader loader(
        [](const AddExternalSubtitleCommand& command) {
            (void)QFile::remove(command.source);
            return std::optional{player::ids::RequestId{71}};
        });
    const MediaGeneration generation{12};
    loader.acceptSnapshot(readySnapshot(generation));

    QVERIFY(loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QVERIFY(loader.loading());
    QVERIFY(!QFileInfo::exists(subtitlePath));

    loader.acceptRequestResult(
        player::ids::RequestId{71},
        generation,
        false,
        QStringLiteral("loading failed after file removal"));
    QVERIFY(!loader.loading());
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("backend-rejected"));
}

void ExternalSubtitleFlowTest::commandFlowUsesGenerationScopedCachedSubAdd()
{
    using namespace player::playback;

    const domain::PlaybackCommand command{
        player::ids::RequestId{41},
        domain::PlaybackCommandPayload{
            domain::AddExternalSubtitleCommand{QStringLiteral("C:/media/captions.ass")}}};
    QVERIFY(!domain::validatePlaybackCommand(command).has_value());

    const auto mapped = mpv::MpvPlaybackCommandMapper::map(command.payload());
    QVERIFY(mapped.has_value());

    QString error;
    const auto encoded = mpv::MpvCommandEncoder::encode(*mapped, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{
            QByteArrayLiteral("sub-add"),
            QByteArrayLiteral("C:/media/captions.ass"),
            QByteArrayLiteral("cached")}));

    player::playback::application::RequestTracker tracker;
    const domain::MediaGeneration generation{7};
    QCOMPARE(
        tracker.track(command, generation),
        player::playback::application::RequestTrackStatus::Tracked);

    const auto firstRecord = tracker.record(command.requestId());
    QVERIFY(firstRecord.has_value());
    QVERIFY(firstRecord->type == player::playback::application::PlaybackRequestType::AddExternalSubtitle);
    QVERIFY(firstRecord->generation.has_value());
    QCOMPARE(firstRecord->generation->value(), generation.value());

    const domain::PlaybackCommand secondCommand{
        player::ids::RequestId{42},
        domain::PlaybackCommandPayload{
            domain::AddExternalSubtitleCommand{QStringLiteral("C:/media/other.srt")}}};
    QCOMPARE(
        tracker.track(secondCommand, generation),
        player::playback::application::RequestTrackStatus::Tracked);
    QCOMPARE(tracker.supersedePendingFor(secondCommand, generation), std::size_t{0});
    QCOMPARE(
        tracker.cancelMediaRequestsForGenerationChange(domain::MediaGeneration{8}),
        std::size_t{2});
}

void ExternalSubtitleFlowTest::successfulReplyRefreshesExternalSubtitleTrackState()
{
    const QString session = readSource(
        QStringLiteral("src/playback/application/session/playback_session.cpp"));
    const QString backend = readSource(
        QStringLiteral("src/playback/application/session/backend/playback_session_backend.cpp"));
    const QString thread = readSource(
        QStringLiteral("src/playback/application/session/playback_session_thread.cpp"));
    const QString composition = readSource(
        QStringLiteral("src/app/composition/playback_composition.cpp"));
    const QString container = readSource(
        QStringLiteral("src/app/composition/application_container.cpp"));
    const QString loader = readSource(
        QStringLiteral("src/tracks/application/external_subtitle_loader.cpp"));

    QVERIFY(!session.isEmpty());
    QVERIFY(!backend.isEmpty());
    QVERIFY(!thread.isEmpty());
    QVERIFY(!composition.isEmpty());
    QVERIFY(!container.isEmpty());
    QVERIFY(!loader.isEmpty());

    QVERIFY(session.contains(QStringLiteral(
        "resolution.record->type == PlaybackRequestType::AddExternalSubtitle")));
    QVERIFY(session.contains(QStringLiteral(
        "backend_->refreshExternalSubtitleTrackState(*resolution.record->generation)")));
    QVERIFY(backend.contains(QStringLiteral("kExternalSubtitleRefreshProperties")));
    QVERIFY(backend.contains(QStringLiteral("MpvPropertyId::TrackList")));
    QVERIFY(backend.contains(QStringLiteral("MpvPropertyId::SelectedSubtitleTrack")));
    QVERIFY(backend.contains(QStringLiteral("kExternalSubtitleRefreshDelaysMilliseconds")));
    QVERIFY(backend.contains(QStringLiteral("QTimer::singleShot")));
    QVERIFY(session.contains(QStringLiteral("publishRequestFinished")));
    QVERIFY(thread.contains(QStringLiteral("&PlaybackSession::requestFinished")));
    QVERIFY(composition.contains(QStringLiteral("requestOutcomeObserver_")));
    QVERIFY(container.contains(QStringLiteral("externalSubtitleLoader_->acceptRequestResult")));
    QVERIFY(container.contains(QStringLiteral("ExternalSubtitleLoader::acceptSnapshot")));
    QVERIFY(thread.contains(QStringLiteral("External subtitle playback request failed:")));
    QVERIFY(loader.contains(QStringLiteral("External subtitle load submitted to playback backend.")));
    QVERIFY(loader.contains(QStringLiteral("External subtitle load deduplicated")));
    QVERIFY(loader.contains(QStringLiteral("External subtitle load rejected:")));
}

void ExternalSubtitleFlowTest::qmlKeepsFilePickingOutsideTrackPopup()
{
    const QString popup = readSource(
        QStringLiteral("src/presentation/qml/features/tracks/TrackSelectionPopup.qml"));
    const QString dialog = readSource(
        QStringLiteral("src/presentation/qml/features/tracks/ExternalSubtitleOpenDialog.qml"));
    const QString utility = readSource(
        QStringLiteral("src/presentation/qml/screens/player/osc/PlayerUtilityControls.qml"));
    const QString screen = readSource(
        QStringLiteral("src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString shell = readSource(
        QStringLiteral("src/presentation/qml/shell/MainWindow.qml"));

    QVERIFY(!popup.isEmpty());
    QVERIFY(!dialog.isEmpty());
    QVERIFY(!utility.isEmpty());
    QVERIFY(!screen.isEmpty());
    QVERIFY(!shell.isEmpty());

    QVERIFY(popup.contains(QStringLiteral("objectName: \"addExternalSubtitleButton\"")));
    QVERIFY(popup.contains(QStringLiteral("signal addExternalSubtitleRequested()")));
    QVERIFY(!popup.contains(QStringLiteral("FileDialog")));
    QVERIFY(!popup.contains(QStringLiteral("mpv_")));

    QVERIFY(dialog.contains(QStringLiteral("FileDialog")));
    QVERIFY(dialog.contains(QStringLiteral("*.srt *.ass")));
    QVERIFY(dialog.contains(QStringLiteral("loadLocalSubtitle(root.selectedFile)")));

    QVERIFY(utility.contains(QStringLiteral("mediaAvailable: false")));
    QVERIFY(utility.contains(QStringLiteral("externalSubtitleAvailable: false")));
    QVERIFY(utility.contains(QStringLiteral("onAddExternalSubtitleRequested")));
    QVERIFY(screen.contains(QStringLiteral("signal openExternalSubtitleRequested()")));
    QVERIFY(shell.contains(QStringLiteral("ExternalSubtitleOpenDialog")));
    QVERIFY(shell.contains(QStringLiteral("externalSubtitleLoader: window.externalSubtitleLoader")));
    QVERIFY(shell.contains(QStringLiteral("|| externalSubtitleOpenDialog.visible")));
    QVERIFY(!shell.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!shell.contains(QStringLiteral("mpv_")));
}

} // namespace player::tracks::application

QTEST_GUILESS_MAIN(player::tracks::application::ExternalSubtitleFlowTest)
#include "external_subtitle_flow_test.moc"
