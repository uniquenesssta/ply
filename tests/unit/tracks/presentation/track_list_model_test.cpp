#include "playback/domain/models/track_descriptor.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"
#include "tracks/presentation/track_list_model.h"

#include <QtTest>

#include <QByteArray>
#include <QList>
#include <QModelIndex>
#include <QString>

#include <optional>
#include <utility>

namespace player::tracks::presentation {
namespace {

using player::playback::domain::MediaGeneration;
using player::playback::domain::PlaybackLifecycleState;
using player::playback::domain::PlaybackSnapshot;
using player::playback::domain::PlaybackSnapshotState;
using player::playback::domain::TrackDescriptor;
using player::playback::domain::TrackKind;

TrackDescriptor makeTrack(
    qint64 id,
    TrackKind kind,
    QString title = {},
    QString language = {},
    QString codec = {})
{
    TrackDescriptor track;
    track.id = id;
    track.kind = kind;
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

PlaybackSnapshot makeSnapshot(
    quint64 generation,
    QList<TrackDescriptor> tracks,
    std::optional<qint64> selectedAudioId = std::nullopt,
    std::optional<qint64> selectedSubtitleId = std::nullopt)
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{generation};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.tracks.tracks = std::move(tracks);
    state.tracks.selectedAudioId = selectedAudioId;
    state.tracks.selectedSubtitleId = selectedSubtitleId;
    return PlaybackSnapshot{std::move(state)};
}

} // namespace

class TrackListModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void separatesAudioAndSubtitleRows();
    void selectionComesFromSnapshotSelectedIds();
    void missingSelectedIdProducesNoFakeSelection();
    void mediaSwitchClearsThenReplacesRows();
    void exposesReadonlyRoleContract();
};

void TrackListModelTest::separatesAudioAndSubtitleRows()
{
    TrackDescriptor commentary = makeTrack(
        3,
        TrackKind::Audio,
        QStringLiteral("Director Commentary"),
        QStringLiteral("eng"),
        QStringLiteral("aac"));
    commentary.defaultTrack = true;

    TrackDescriptor subtitle = makeTrack(
        7,
        TrackKind::Subtitle,
        QStringLiteral("Signs & Songs"),
        QStringLiteral("jpn"),
        QStringLiteral("ass"));
    subtitle.forced = true;
    subtitle.external = true;
    subtitle.externalFilename = QStringLiteral("C:/media/subs/signs.ass");

    const PlaybackSnapshot snapshot = makeSnapshot(
        1,
        QList<TrackDescriptor>{
            makeTrack(1, TrackKind::Video),
            makeTrack(2, TrackKind::Audio, QStringLiteral("Main Audio"), QStringLiteral("eng")),
            commentary,
            subtitle,
        },
        qint64{3},
        qint64{7});

    TrackListModel audioModel(TrackKind::Audio);
    TrackListModel subtitleModel(TrackKind::Subtitle);
    audioModel.acceptSnapshot(snapshot);
    subtitleModel.acceptSnapshot(snapshot);

    QCOMPARE(audioModel.count(), 2);
    QCOMPARE(subtitleModel.count(), 1);
    QCOMPARE(audioModel.selectedTrackId(), qint64{3});
    QCOMPARE(subtitleModel.selectedTrackId(), qint64{7});

    const QModelIndex commentaryIndex = audioModel.index(1, 0);
    QCOMPARE(audioModel.data(commentaryIndex, TrackListModel::TrackIdRole).toLongLong(), qint64{3});
    QCOMPARE(
        audioModel.data(commentaryIndex, TrackListModel::TitleRole).toString(),
        QStringLiteral("Director Commentary"));
    QCOMPARE(
        audioModel.data(commentaryIndex, TrackListModel::LanguageRole).toString(),
        QStringLiteral("eng"));
    QCOMPARE(
        audioModel.data(commentaryIndex, TrackListModel::CodecRole).toString(),
        QStringLiteral("aac"));
    QVERIFY(audioModel.data(commentaryIndex, TrackListModel::SelectedRole).toBool());
    QVERIFY(audioModel.data(commentaryIndex, TrackListModel::DefaultRole).toBool());

    const QModelIndex subtitleIndex = subtitleModel.index(0, 0);
    QCOMPARE(subtitleModel.data(subtitleIndex, TrackListModel::TrackIdRole).toLongLong(), qint64{7});
    QVERIFY(subtitleModel.data(subtitleIndex, TrackListModel::SelectedRole).toBool());
    QVERIFY(subtitleModel.data(subtitleIndex, TrackListModel::ForcedRole).toBool());
    QVERIFY(subtitleModel.data(subtitleIndex, TrackListModel::ExternalRole).toBool());
    QCOMPARE(
        subtitleModel.data(subtitleIndex, TrackListModel::ExternalFilenameRole).toString(),
        QStringLiteral("C:/media/subs/signs.ass"));
}

void TrackListModelTest::selectionComesFromSnapshotSelectedIds()
{
    TrackDescriptor first = makeTrack(2, TrackKind::Audio, QStringLiteral("First"));
    TrackDescriptor second = makeTrack(3, TrackKind::Audio, QStringLiteral("Second"));
    first.selected = true;
    second.selected = false;

    TrackListModel model(TrackKind::Audio);
    model.acceptSnapshot(makeSnapshot(
        1,
        QList<TrackDescriptor>{first, second},
        qint64{3}));

    QCOMPARE(model.selectedTrackId(), qint64{3});
    QVERIFY(!model.data(model.index(0, 0), TrackListModel::SelectedRole).toBool());
    QVERIFY(model.data(model.index(1, 0), TrackListModel::SelectedRole).toBool());

    model.acceptSnapshot(makeSnapshot(
        1,
        QList<TrackDescriptor>{first, second},
        qint64{2}));

    QCOMPARE(model.selectedTrackId(), qint64{2});
    QVERIFY(model.data(model.index(0, 0), TrackListModel::SelectedRole).toBool());
    QVERIFY(!model.data(model.index(1, 0), TrackListModel::SelectedRole).toBool());
}

void TrackListModelTest::missingSelectedIdProducesNoFakeSelection()
{
    TrackDescriptor first = makeTrack(2, TrackKind::Audio, QStringLiteral("First"));
    TrackDescriptor second = makeTrack(3, TrackKind::Audio, QStringLiteral("Second"));
    first.selected = true;

    TrackListModel model(TrackKind::Audio);
    model.acceptSnapshot(makeSnapshot(
        1,
        QList<TrackDescriptor>{first, second},
        qint64{99}));

    QCOMPARE(model.count(), 2);
    QCOMPARE(model.selectedTrackId(), qint64{0});
    QVERIFY(!model.data(model.index(0, 0), TrackListModel::SelectedRole).toBool());
    QVERIFY(!model.data(model.index(1, 0), TrackListModel::SelectedRole).toBool());
}

void TrackListModelTest::mediaSwitchClearsThenReplacesRows()
{
    TrackListModel audioModel(TrackKind::Audio);
    TrackListModel subtitleModel(TrackKind::Subtitle);

    const PlaybackSnapshot readyA = makeSnapshot(
        1,
        QList<TrackDescriptor>{makeTrack(2, TrackKind::Audio), makeTrack(7, TrackKind::Subtitle)},
        qint64{2},
        qint64{7});
    audioModel.acceptSnapshot(readyA);
    subtitleModel.acceptSnapshot(readyA);

    QCOMPARE(audioModel.count(), 1);
    QCOMPARE(subtitleModel.count(), 1);
    QCOMPARE(audioModel.selectedTrackId(), qint64{2});
    QCOMPARE(subtitleModel.selectedTrackId(), qint64{7});

    const PlaybackSnapshot openingB = PlaybackSnapshot::opening(
        MediaGeneration{2},
        QStringLiteral("file:///B.mkv"));
    audioModel.acceptSnapshot(openingB);
    subtitleModel.acceptSnapshot(openingB);

    QCOMPARE(audioModel.count(), 0);
    QCOMPARE(subtitleModel.count(), 0);
    QCOMPARE(audioModel.selectedTrackId(), qint64{0});
    QCOMPARE(subtitleModel.selectedTrackId(), qint64{0});

    const PlaybackSnapshot readyB = makeSnapshot(
        2,
        QList<TrackDescriptor>{makeTrack(11, TrackKind::Audio, QStringLiteral("B Audio"))},
        qint64{11});
    audioModel.acceptSnapshot(readyB);
    subtitleModel.acceptSnapshot(readyB);

    QCOMPARE(audioModel.count(), 1);
    QCOMPARE(subtitleModel.count(), 0);
    QCOMPARE(audioModel.selectedTrackId(), qint64{11});
    QCOMPARE(subtitleModel.selectedTrackId(), qint64{0});
    QCOMPARE(
        audioModel.data(audioModel.index(0, 0), TrackListModel::TrackIdRole).toLongLong(),
        qint64{11});
}

void TrackListModelTest::exposesReadonlyRoleContract()
{
    TrackListModel model(TrackKind::Audio);
    model.acceptSnapshot(makeSnapshot(
        1,
        QList<TrackDescriptor>{makeTrack(2, TrackKind::Audio)}));

    const auto roles = model.roleNames();
    QCOMPARE(roles.value(TrackListModel::TrackIdRole), QByteArrayLiteral("trackId"));
    QCOMPARE(roles.value(TrackListModel::TitleRole), QByteArrayLiteral("title"));
    QCOMPARE(roles.value(TrackListModel::LanguageRole), QByteArrayLiteral("language"));
    QCOMPARE(roles.value(TrackListModel::CodecRole), QByteArrayLiteral("codec"));
    QCOMPARE(roles.value(TrackListModel::SelectedRole), QByteArrayLiteral("selected"));
    QCOMPARE(roles.value(TrackListModel::DefaultRole), QByteArrayLiteral("defaultTrack"));
    QCOMPARE(roles.value(TrackListModel::ForcedRole), QByteArrayLiteral("forced"));
    QCOMPARE(roles.value(TrackListModel::ExternalRole), QByteArrayLiteral("external"));
    QCOMPARE(
        roles.value(TrackListModel::ExternalFilenameRole),
        QByteArrayLiteral("externalFilename"));

    const QModelIndex index = model.index(0, 0);
    QVERIFY(model.flags(index).testFlag(Qt::ItemIsEnabled));
    QVERIFY(model.flags(index).testFlag(Qt::ItemIsSelectable));
    QVERIFY(!model.flags(index).testFlag(Qt::ItemIsEditable));
}

} // namespace player::tracks::presentation

QTEST_GUILESS_MAIN(player::tracks::presentation::TrackListModelTest)
#include "track_list_model_test.moc"
