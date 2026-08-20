#include "chapters/presentation/chapter_model.h"
#include "playback/domain/models/chapter_descriptor.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QtTest>

#include <QByteArray>
#include <QList>
#include <QModelIndex>
#include <QString>

#include <optional>
#include <utility>

namespace player::chapters::presentation {
namespace {

using player::playback::domain::ChapterDescriptor;
using player::playback::domain::MediaGeneration;
using player::playback::domain::PlaybackLifecycleState;
using player::playback::domain::PlaybackSnapshot;
using player::playback::domain::PlaybackSnapshotState;

ChapterDescriptor makeChapter(
    qsizetype index,
    double startSeconds,
    std::optional<QString> title = std::nullopt)
{
    ChapterDescriptor chapter;
    chapter.index = index;
    chapter.startSeconds = startSeconds;
    chapter.title = std::move(title);
    return chapter;
}

PlaybackSnapshot makeSnapshot(
    quint64 generation,
    QList<ChapterDescriptor> chapters)
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{generation};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.chapters.chapters = std::move(chapters);
    state.capabilities.hasChapters = !state.chapters.chapters.isEmpty();
    return PlaybackSnapshot{std::move(state)};
}

} // namespace

class ChapterModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void emptySnapshotProducesStableEmptyModel();
    void mapsSnapshotRowsDeterministically();
    void preservesDuplicateTimesLongTitlesAndMissingTitle();
    void mediaSwitchClearsThenReplacesRows();
    void exposesReadonlyRoleContract();
};

void ChapterModelTest::emptySnapshotProducesStableEmptyModel()
{
    ChapterModel model;
    model.acceptSnapshot(makeSnapshot(1, {}));

    QCOMPARE(model.count(), 0);
    QCOMPARE(model.rowCount(), 0);
}

void ChapterModelTest::mapsSnapshotRowsDeterministically()
{
    ChapterModel model;
    model.acceptSnapshot(makeSnapshot(
        1,
        QList<ChapterDescriptor>{
            makeChapter(0, 0.0, QStringLiteral("Opening")),
            makeChapter(1, 37.25, QStringLiteral("Middle")),
            makeChapter(2, 120.0, QStringLiteral("End")),
        }));

    QCOMPARE(model.count(), 3);

    const QModelIndex middle = model.index(1, 0);
    QCOMPARE(model.data(middle, ChapterModel::IndexRole).toLongLong(), qint64{1});
    QCOMPARE(model.data(middle, ChapterModel::TitleRole).toString(), QStringLiteral("Middle"));
    QCOMPARE(model.data(middle, ChapterModel::TimeRole).toDouble(), 37.25);
    QCOMPARE(
        model.data(middle, ChapterModel::TimeTextRole).toString(),
        QStringLiteral("00:00:37"));
}

void ChapterModelTest::preservesDuplicateTimesLongTitlesAndMissingTitle()
{
    const QString longTitle = QStringLiteral(
        "A very long chapter title that belongs to the backend-derived snapshot and must remain "
        "unchanged by the read-only presentation model regardless of its display length.");

    ChapterModel model;
    model.acceptSnapshot(makeSnapshot(
        1,
        QList<ChapterDescriptor>{
            makeChapter(4, 42.5, longTitle),
            makeChapter(5, 42.5),
        }));

    QCOMPARE(model.count(), 2);
    QCOMPARE(model.data(model.index(0, 0), ChapterModel::IndexRole).toLongLong(), qint64{4});
    QCOMPARE(model.data(model.index(0, 0), ChapterModel::TitleRole).toString(), longTitle);
    QCOMPARE(model.data(model.index(0, 0), ChapterModel::TimeRole).toDouble(), 42.5);
    QCOMPARE(model.data(model.index(1, 0), ChapterModel::IndexRole).toLongLong(), qint64{5});
    QCOMPARE(
        model.data(model.index(1, 0), ChapterModel::TitleRole).toString(),
        QStringLiteral("Chapter 6"));
    QCOMPARE(model.data(model.index(1, 0), ChapterModel::TimeRole).toDouble(), 42.5);
}

void ChapterModelTest::mediaSwitchClearsThenReplacesRows()
{
    ChapterModel model;
    model.acceptSnapshot(makeSnapshot(
        1,
        QList<ChapterDescriptor>{makeChapter(0, 0.0, QStringLiteral("A"))}));
    QCOMPARE(model.count(), 1);

    model.acceptSnapshot(PlaybackSnapshot::opening(
        MediaGeneration{2},
        QStringLiteral("file:///B.mkv")));
    QCOMPARE(model.count(), 0);

    model.acceptSnapshot(makeSnapshot(
        2,
        QList<ChapterDescriptor>{
            makeChapter(0, 0.0, QStringLiteral("B1")),
            makeChapter(1, 15.0, QStringLiteral("B2")),
        }));
    QCOMPARE(model.count(), 2);
    QCOMPARE(model.data(model.index(1, 0), ChapterModel::TitleRole).toString(), QStringLiteral("B2"));
}

void ChapterModelTest::exposesReadonlyRoleContract()
{
    ChapterModel model;
    model.acceptSnapshot(makeSnapshot(
        1,
        QList<ChapterDescriptor>{makeChapter(0, 0.0, QStringLiteral("Opening"))}));

    const auto roles = model.roleNames();
    QCOMPARE(roles.value(ChapterModel::IndexRole), QByteArrayLiteral("index"));
    QCOMPARE(roles.value(ChapterModel::TitleRole), QByteArrayLiteral("title"));
    QCOMPARE(roles.value(ChapterModel::TimeRole), QByteArrayLiteral("time"));
    QCOMPARE(roles.value(ChapterModel::TimeTextRole), QByteArrayLiteral("timeText"));

    const QModelIndex index = model.index(0, 0);
    QVERIFY(model.flags(index).testFlag(Qt::ItemIsEnabled));
    QVERIFY(model.flags(index).testFlag(Qt::ItemIsSelectable));
    QVERIFY(!model.flags(index).testFlag(Qt::ItemIsEditable));
}

} // namespace player::chapters::presentation

QTEST_GUILESS_MAIN(player::chapters::presentation::ChapterModelTest)
#include "chapter_model_test.moc"
