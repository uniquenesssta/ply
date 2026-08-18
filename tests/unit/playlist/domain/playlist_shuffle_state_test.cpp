#include "media/domain/media_source.h"
#include "playlist/domain/playlist_entry.h"
#include "playlist/domain/playlist_shuffle_state.h"

#include <QSet>
#include <QtTest>

#include <algorithm>
#include <optional>
#include <vector>

namespace player::playlist::domain {
namespace {

PlaylistEntry entry(quint64 id, const QString& name)
{
    return PlaylistEntry{
        PlaylistEntryId{id},
        media::domain::MediaSource::localFile(QStringLiteral("C:/media/") + name)};
}

std::vector<PlaylistEntry> fourEntries()
{
    return {
        entry(1, QStringLiteral("a.mp4")),
        entry(2, QStringLiteral("b.mp4")),
        entry(3, QStringLiteral("c.mp4")),
        entry(4, QStringLiteral("d.mp4")),
    };
}

} // namespace

class PlaylistShuffleStateTest final : public QObject
{
    Q_OBJECT

private slots:
    void disabledStateReturnsNoCandidate();
    void previewDoesNotMutateUntilSelectionCommit();
    void cycleVisitsEveryOtherEntryOnceBeforeStopping();
    void repeatCycleRestartsWithoutImmediateCurrent();
    void activeCycleAcceptsAppendAndRemovalUpdates();
};

void PlaylistShuffleStateTest::disabledStateReturnsNoCandidate()
{
    PlaylistShuffleState state;
    const auto entries = fourEntries();

    QVERIFY(!state.enabled());
    QVERIFY(!state.takeNext(entries, PlaylistEntryId{1}, false).has_value());
    QVERIFY(!state.previewNext(entries, PlaylistEntryId{1}, false).has_value());
}

void PlaylistShuffleStateTest::previewDoesNotMutateUntilSelectionCommit()
{
    PlaylistShuffleState state;
    const auto entries = fourEntries();
    state.setEnabled(true);

    const auto candidate = state.previewNext(entries, PlaylistEntryId{1}, false);
    QVERIFY(candidate.has_value());
    QVERIFY(candidate->value() != quint64{1});
    QVERIFY(!state.cycleInitialized());
    QVERIFY(state.remainingEntryIds().empty());

    QVERIFY(state.commitNextSelection(
        entries,
        PlaylistEntryId{1},
        *candidate,
        false));
    QVERIFY(state.cycleInitialized());
    QCOMPARE(state.remainingEntryIds().size(), std::size_t{2});
    QVERIFY(std::find(
        state.remainingEntryIds().cbegin(),
        state.remainingEntryIds().cend(),
        *candidate) == state.remainingEntryIds().cend());
}

void PlaylistShuffleStateTest::cycleVisitsEveryOtherEntryOnceBeforeStopping()
{
    PlaylistShuffleState state;
    const auto entries = fourEntries();
    state.setEnabled(true);

    std::optional<PlaylistEntryId> current = PlaylistEntryId{1};
    QSet<quint64> visited;
    for (int index = 0; index < 3; ++index) {
        const auto next = state.takeNext(entries, current, false);
        QVERIFY(next.has_value());
        QVERIFY(*next != *current);
        QVERIFY(!visited.contains(next->value()));
        visited.insert(next->value());
        state.onEntrySelected(*next);
        current = *next;
    }

    QCOMPARE(visited.size(), 3);
    QVERIFY(!state.takeNext(entries, current, false).has_value());
}

void PlaylistShuffleStateTest::repeatCycleRestartsWithoutImmediateCurrent()
{
    PlaylistShuffleState state;
    const auto entries = fourEntries();
    state.setEnabled(true);

    std::optional<PlaylistEntryId> current = PlaylistEntryId{1};
    for (int index = 0; index < 3; ++index) {
        const auto next = state.takeNext(entries, current, false);
        QVERIFY(next.has_value());
        state.onEntrySelected(*next);
        current = *next;
    }

    const auto restarted = state.takeNext(entries, current, true);
    QVERIFY(restarted.has_value());
    QVERIFY(*restarted != *current);
}

void PlaylistShuffleStateTest::activeCycleAcceptsAppendAndRemovalUpdates()
{
    PlaylistShuffleState state;
    std::vector<PlaylistEntry> entries{
        entry(1, QStringLiteral("a.mp4")),
        entry(2, QStringLiteral("b.mp4")),
    };
    state.setEnabled(true);

    std::optional<PlaylistEntryId> current = PlaylistEntryId{1};
    const auto second = state.takeNext(entries, current, false);
    QVERIFY(second.has_value());
    QCOMPARE(second->value(), quint64{2});
    state.onEntrySelected(*second);
    current = *second;

    entries.push_back(entry(3, QStringLiteral("c.mp4")));
    state.onEntryAdded(PlaylistEntryId{3});
    const auto appended = state.takeNext(entries, current, false);
    QVERIFY(appended.has_value());
    QCOMPARE(appended->value(), quint64{3});

    state.resetCycle();
    current = PlaylistEntryId{1};
    const auto candidate = state.takeNext(entries, current, false);
    QVERIFY(candidate.has_value());
    const PlaylistEntryId other = candidate->value() == 2
        ? PlaylistEntryId{3}
        : PlaylistEntryId{2};
    state.onEntrySelected(*candidate);
    current = *candidate;
    state.onEntryRemoved(other);

    QVERIFY(!state.takeNext(entries, current, false).has_value());
}

} // namespace player::playlist::domain

QTEST_GUILESS_MAIN(player::playlist::domain::PlaylistShuffleStateTest)
#include "playlist_shuffle_state_test.moc"
