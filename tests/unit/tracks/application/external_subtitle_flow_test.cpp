#include "foundation/ids/request_id.h"
#include "playback/application/requests/request_tracker.h"
#include "playback/domain/commands/external_subtitle_command.h"
#include "playback/domain/commands/playback_command.h"
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

} // namespace

class ExternalSubtitleFlowTest final : public QObject
{
    Q_OBJECT

private slots:
    void loaderAcceptsReadableSrtAndAss();
    void loaderRejectsInvalidLocalSources();
    void loaderReportsSubmissionFailure();
    void commandFlowUsesGenerationScopedCachedSubAdd();
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
    ExternalSubtitleLoader loader(
        [&submittedSources](const player::playback::domain::AddExternalSubtitleCommand& command) {
            submittedSources.append(command.source);
            return true;
        });

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
            return true;
        });

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
            return false;
        });

    QVERIFY(!loader.loadLocalSubtitle(QUrl::fromLocalFile(subtitlePath)));
    QCOMPARE(loader.lastErrorKey(), QStringLiteral("submission-failed"));
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
    QVERIFY(!shell.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!shell.contains(QStringLiteral("mpv_")));
}

} // namespace player::tracks::application

QTEST_GUILESS_MAIN(player::tracks::application::ExternalSubtitleFlowTest)
#include "external_subtitle_flow_test.moc"
