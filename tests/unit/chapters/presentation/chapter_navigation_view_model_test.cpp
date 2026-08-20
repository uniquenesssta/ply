#include "chapters/presentation/chapter_navigation_view_model.h"
#include "playback/domain/models/chapter_descriptor.h"
#include "playback/domain/state/playback_lifecycle_state.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QtTest>

#include <QList>
#include <QSignalSpy>
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
    double positionSeconds,
    bool seekable = true)
{
    PlaybackSnapshotState state;
    state.generation = MediaGeneration{generation};
    state.lifecycle = PlaybackLifecycleState::Ready;
    state.timeline.positionSeconds = positionSeconds;
    state.timeline.durationSeconds = 90.0;
    state.timeline.seekable = seekable;
    state.chapters.chapters = {
        makeChapter(0, 0.0, QStringLiteral("Opening")),
        makeChapter(1, 10.0, QStringLiteral("Middle")),
        makeChapter(2, 10.0),
        makeChapter(3, 40.0, QStringLiteral("Finale")),
    };
    state.capabilities.hasChapters = true;
    return PlaybackSnapshot{std::move(state)};
}

} // namespace

class ChapterNavigationViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void projectsCurrentChapterFromSnapshotPosition();
    void chapterClickEmitsAbsoluteTargetWithoutChangingCurrentProjection();
    void rejectsNonSeekableAndUnknownChapterRequests();
    void previousAndNextUseSnapshotChapterOrder();
    void requestFailureAndMediaSwitchClearPendingTarget();
};

void ChapterNavigationViewModelTest::projectsCurrentChapterFromSnapshotPosition()
{
    ChapterNavigationViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(1, 12.0));

    QVERIFY(viewModel.canSeek());
    QCOMPARE(viewModel.chapterCount(), 4);
    QCOMPARE(viewModel.currentChapterIndex(), qint64{2});
    QCOMPARE(viewModel.currentPosition(), 3);
    QCOMPARE(viewModel.currentChapterTitle(), QStringLiteral("Chapter 3"));
    QVERIFY(viewModel.canSeekPrevious());
    QVERIFY(viewModel.canSeekNext());
    QCOMPARE(viewModel.pendingChapterIndex(), qint64{-1});
}

void ChapterNavigationViewModelTest::chapterClickEmitsAbsoluteTargetWithoutChangingCurrentProjection()
{
    ChapterNavigationViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(1, 1.0));
    QSignalSpy seekSpy(&viewModel, &ChapterNavigationViewModel::seekRequested);

    QVERIFY(viewModel.requestChapterSeek(3));
    QCOMPARE(seekSpy.size(), 1);
    QCOMPARE(seekSpy.constFirst().constFirst().toDouble(), 40.0);
    QCOMPARE(viewModel.currentChapterIndex(), qint64{0});
    QCOMPARE(viewModel.pendingChapterIndex(), qint64{3});

    viewModel.acceptSnapshot(makeSnapshot(1, 40.4));
    QCOMPARE(viewModel.currentChapterIndex(), qint64{3});
    QCOMPARE(viewModel.pendingChapterIndex(), qint64{-1});
}

void ChapterNavigationViewModelTest::rejectsNonSeekableAndUnknownChapterRequests()
{
    ChapterNavigationViewModel viewModel;
    QSignalSpy seekSpy(&viewModel, &ChapterNavigationViewModel::seekRequested);

    viewModel.acceptSnapshot(makeSnapshot(1, 12.0, false));
    QVERIFY(!viewModel.canSeek());
    QVERIFY(!viewModel.requestChapterSeek(1));
    QCOMPARE(seekSpy.size(), 0);

    viewModel.acceptSnapshot(makeSnapshot(1, 12.0, true));
    QVERIFY(!viewModel.requestChapterSeek(99));
    QVERIFY(!viewModel.requestChapterSeek(-1));
    QCOMPARE(seekSpy.size(), 0);
}

void ChapterNavigationViewModelTest::previousAndNextUseSnapshotChapterOrder()
{
    ChapterNavigationViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(1, 12.0));
    QSignalSpy seekSpy(&viewModel, &ChapterNavigationViewModel::seekRequested);

    QVERIFY(viewModel.requestPreviousChapter());
    QCOMPARE(seekSpy.constLast().constFirst().toDouble(), 10.0);
    QVERIFY(viewModel.rejectPendingSeek());

    QVERIFY(viewModel.requestNextChapter());
    QCOMPARE(seekSpy.constLast().constFirst().toDouble(), 40.0);

    viewModel.acceptSnapshot(makeSnapshot(1, 0.0));
    QVERIFY(!viewModel.canSeekPrevious());
    QVERIFY(!viewModel.requestPreviousChapter());

    viewModel.acceptSnapshot(makeSnapshot(1, 45.0));
    QVERIFY(!viewModel.canSeekNext());
    QVERIFY(!viewModel.requestNextChapter());
}

void ChapterNavigationViewModelTest::requestFailureAndMediaSwitchClearPendingTarget()
{
    ChapterNavigationViewModel viewModel;
    viewModel.acceptSnapshot(makeSnapshot(1, 1.0));

    QVERIFY(viewModel.requestChapterSeek(1));
    QCOMPARE(viewModel.pendingChapterIndex(), qint64{1});
    QVERIFY(viewModel.rejectPendingSeek());
    QCOMPARE(viewModel.pendingChapterIndex(), qint64{-1});
    QVERIFY(!viewModel.rejectPendingSeek());

    QVERIFY(viewModel.requestChapterSeek(3));
    viewModel.acceptSnapshot(PlaybackSnapshot::opening(
        MediaGeneration{2},
        QStringLiteral("file:///next.mkv")));
    QVERIFY(!viewModel.canSeek());
    QCOMPARE(viewModel.chapterCount(), 0);
    QCOMPARE(viewModel.currentChapterIndex(), qint64{-1});
    QCOMPARE(viewModel.pendingChapterIndex(), qint64{-1});
}

} // namespace player::chapters::presentation

QTEST_GUILESS_MAIN(player::chapters::presentation::ChapterNavigationViewModelTest)
#include "chapter_navigation_view_model_test.moc"
