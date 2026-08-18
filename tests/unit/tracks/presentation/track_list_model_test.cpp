#include "playback/domain/state/playback_snapshot.h"
#include "tracks/presentation/chapter_list_model.h"
#include "tracks/presentation/track_list_model.h"

#include <QtTest>

namespace player::tracks::presentation {
namespace {

using namespace player::playback::domain;

PlaybackSnapshot snapshotWithTracksAndChapters(
    const QList<TrackDescriptor>& tracks,
    const QList<ChapterDescriptor>& chapters = {})
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{9};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.tracks.tracks = tracks;
    state.chapters.chapters = chapters;
    return PlaybackSnapshot{std::move(state)};
}

TrackDescriptor audioTrack(
    qint64 id,
    bool selected,
    QString title = {},
    QString language = {},
    QString codec = {})
{
    TrackDescriptor track;
    track.id = id;
    track.kind = TrackKind::Audio;
    track.selected = selected;
    if (!title.isEmpty()) {
        track.title = std::move(title);
    }
    if (!language.isEmpty()) {
        track.language = std::move(language);
    }
    if (!codec.isEmpty()) {
        track.codec = std::move(codec);
    }
    return track;
}

TrackDescriptor subtitleTrack(
    qint64 id,
    bool selected,
    bool external = false,
    QString externalFilename = {})
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

class TrackListModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void startsEmpty();
    void projectsOnlyRequestedKind();
    void projectsStableIdsAndSelectedRow();
    void replacesWholeTableOnMediaSwitch();
    void fallsBackToLanguageAndCodecAndFilename();
    void chaptersProjectIndexTitleAndStart();
    void chaptersFallbackTitleAndClearOnSwitch();
};

void TrackListModelTest::startsEmpty()
{
    TrackListModel audioModel(TrackKind::Audio);
    QCOMPARE(audioModel.count(), 0);
    QCOMPARE(audioModel.rowCount(), 0);
    QCOMPARE(audioModel.selectedRow(), -1);
    QCOMPARE(audioModel.flags(QModelIndex{}), Qt::ItemFlags{});
    QCOMPARE(audioModel.kind(), TrackKind::Audio);
}

void TrackListModelTest::projectsOnlyRequestedKind()
{
    TrackListModel audioModel(TrackKind::Audio);
    TrackListModel subtitleModel(TrackKind::Subtitle);

    const PlaybackSnapshot snapshot = snapshotWithTracksAndChapters({
        audioTrack(2, true, QStringLiteral("English"), QStringLiteral("eng"), QStringLiteral("aac")),
        subtitleTrack(5, true, true, QStringLiteral("C:/subs/en.srt")),
        audioTrack(3, false, {}, QStringLiteral("jpn"), {}),
    });

    audioModel.acceptSnapshot(snapshot);
    subtitleModel.acceptSnapshot(snapshot);

    QCOMPARE(audioModel.count(), 2);
    QCOMPARE(subtitleModel.count(), 1);

    const QModelIndex firstAudio = audioModel.index(0, 0);
    QCOMPARE(audioModel.data(firstAudio, TrackListModel::TrackIdRole).toLongLong(), qint64{2});
    QCOMPARE(audioModel.data(firstAudio, TrackListModel::TitleRole).toString(), QStringLiteral("English"));
    QCOMPARE(audioModel.data(firstAudio, TrackListModel::LanguageRole).toString(), QStringLiteral("eng"));
    QCOMPARE(audioModel.data(firstAudio, TrackListModel::CodecRole).toString(), QStringLiteral("aac"));
    QVERIFY(audioModel.data(firstAudio, TrackListModel::SelectedRole).toBool());

    const QModelIndex secondAudio = audioModel.index(1, 0);
    QCOMPARE(audioModel.data(secondAudio, TrackListModel::TrackIdRole).toLongLong(), qint64{3});
    QVERIFY(!audioModel.data(secondAudio, TrackListModel::SelectedRole).toBool());

    const QModelIndex subtitle = subtitleModel.index(0, 0);
    QCOMPARE(subtitleModel.data(subtitle, TrackListModel::TrackIdRole).toLongLong(), qint64{5});
    QVERIFY(subtitleModel.data(subtitle, TrackListModel::ExternalRole).toBool());
    QCOMPARE(
        subtitleModel.data(subtitle, TrackListModel::ExternalFilenameRole).toString(),
        QStringLiteral("C:/subs/en.srt"));
}

void TrackListModelTest::projectsStableIdsAndSelectedRow()
{
    TrackListModel subtitleModel(TrackKind::Subtitle);

    PlaybackSnapshot snapshot = snapshotWithTracksAndChapters({
        subtitleTrack(7, false, true, QStringLiteral("C:/subs/a.srt")),
        subtitleTrack(8, true, true, QStringLiteral("C:/subs/b.srt")),
    });
    subtitleModel.acceptSnapshot(snapshot);
    QCOMPARE(subtitleModel.selectedRow(), 1);

    // Same ids, different order: selected row follows the id, not the index.
    snapshot = snapshotWithTracksAndChapters({
        subtitleTrack(8, true, true, QStringLiteral("C:/subs/b.srt")),
        subtitleTrack(7, false, true, QStringLiteral("C:/subs/a.srt")),
    });
    subtitleModel.acceptSnapshot(snapshot);
    QCOMPARE(subtitleModel.selectedRow(), 0);
    QCOMPARE(
        subtitleModel.data(subtitleModel.index(0, 0), TrackListModel::TrackIdRole).toLongLong(),
        qint64{8});
}

void TrackListModelTest::replacesWholeTableOnMediaSwitch()
{
    TrackListModel audioModel(TrackKind::Audio);

    audioModel.acceptSnapshot(snapshotWithTracksAndChapters({audioTrack(1, true)}));
    QCOMPARE(audioModel.count(), 1);

    // New media without audio tracks must not retain the old rows.
    audioModel.acceptSnapshot(PlaybackSnapshot::stopped(MediaGeneration{10}));
    QCOMPARE(audioModel.count(), 0);
    QCOMPARE(audioModel.selectedRow(), -1);
}

void TrackListModelTest::fallsBackToLanguageAndCodecAndFilename()
{
    TrackListModel audioModel(TrackKind::Audio);
    TrackListModel subtitleModel(TrackKind::Subtitle);

    const PlaybackSnapshot snapshot = snapshotWithTracksAndChapters({
        audioTrack(1, false, {}, QStringLiteral("jpn"), {}),
        audioTrack(2, false, {}, {}, QStringLiteral("opus")),
        subtitleTrack(3, false, true, QStringLiteral("C:/subs/movie.ass")),
    });

    audioModel.acceptSnapshot(snapshot);
    subtitleModel.acceptSnapshot(snapshot);

    QCOMPARE(audioModel.data(audioModel.index(0, 0), TrackListModel::TitleRole).toString(), QStringLiteral("jpn"));
    QCOMPARE(audioModel.data(audioModel.index(1, 0), TrackListModel::TitleRole).toString(), QStringLiteral("opus"));
    QCOMPARE(
        subtitleModel.data(subtitleModel.index(0, 0), TrackListModel::TitleRole).toString(),
        QStringLiteral("movie.ass"));
}

void TrackListModelTest::chaptersProjectIndexTitleAndStart()
{
    ChapterListModel model;

    ChapterDescriptor first;
    first.index = 0;
    first.startSeconds = 0.0;
    first.title = QStringLiteral("Intro");
    ChapterDescriptor second;
    second.index = 1;
    second.startSeconds = 95.5;

    model.acceptSnapshot(snapshotWithTracksAndChapters({}, {first, second}));

    QCOMPARE(model.count(), 2);
    QCOMPARE(model.data(model.index(0, 0), ChapterListModel::ChapterIndexRole).toInt(), 0);
    QCOMPARE(model.data(model.index(0, 0), ChapterListModel::TitleRole).toString(), QStringLiteral("Intro"));
    QCOMPARE(model.data(model.index(0, 0), ChapterListModel::StartSecondsRole).toDouble(), 0.0);
    QCOMPARE(model.data(model.index(1, 0), ChapterListModel::ChapterIndexRole).toInt(), 1);
    QCOMPARE(model.data(model.index(1, 0), ChapterListModel::TitleRole).toString(), QStringLiteral("Chapter 2"));
    QCOMPARE(model.data(model.index(1, 0), ChapterListModel::StartSecondsRole).toDouble(), 95.5);
}

void TrackListModelTest::chaptersFallbackTitleAndClearOnSwitch()
{
    ChapterListModel model;
    QCOMPARE(model.flags(QModelIndex{}), Qt::ItemFlags{});

    ChapterDescriptor only;
    only.index = 0;
    only.startSeconds = 12.0;
    model.acceptSnapshot(snapshotWithTracksAndChapters({}, {only}));
    QCOMPARE(model.count(), 1);
    QCOMPARE(model.data(model.index(0, 0), ChapterListModel::TitleRole).toString(), QStringLiteral("Chapter 1"));

    model.acceptSnapshot(PlaybackSnapshot::stopped(MediaGeneration{11}));
    QCOMPARE(model.count(), 0);
}

} // namespace player::tracks::presentation

QTEST_GUILESS_MAIN(player::tracks::presentation::TrackListModelTest)
#include "track_list_model_test.moc"
