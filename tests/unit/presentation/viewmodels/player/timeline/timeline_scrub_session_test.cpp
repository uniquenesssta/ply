#include "presentation/viewmodels/player/timeline/timeline_scrub_session.h"

#include <QtTest>

#include <cmath>
#include <limits>

namespace player::presentation {
namespace {

using player::playback::domain::MediaGeneration;

bool fuzzyEqual(double left, double right)
{
    return std::abs(left - right) <= 0.000001;
}

} // namespace

class TimelineScrubSessionTest final : public QObject
{
    Q_OBJECT

private slots:
    void dragCommitKeepsPreviewPending();
    void cancelReturnsToIdle();
    void generationChangeCancelsInteraction();
    void invalidInputIsRejectedAndFiniteValuesClamp();
};

void TimelineScrubSessionTest::dragCommitKeepsPreviewPending()
{
    TimelineScrubSession session;
    QVERIFY(session.phase() == TimelineScrubPhase::Idle);
    QVERIFY(!session.isActive());

    QVERIFY(session.begin(MediaGeneration{1}, 0.2));
    QVERIFY(session.isScrubbing());
    QVERIFY(fuzzyEqual(session.previewNormalized(), 0.2));

    QVERIFY(session.update(0.8));
    QVERIFY(fuzzyEqual(session.previewNormalized(), 0.8));

    const std::optional<double> committed = session.commit(0.75);
    QVERIFY(committed.has_value());
    QVERIFY(fuzzyEqual(*committed, 0.75));
    QVERIFY(session.isPendingCommit());
    QVERIFY(session.isActive());
    QVERIFY(fuzzyEqual(session.previewNormalized(), 0.75));

    QVERIFY(session.acknowledgePending());
    QVERIFY(session.phase() == TimelineScrubPhase::Idle);
    QVERIFY(!session.isActive());
}

void TimelineScrubSessionTest::cancelReturnsToIdle()
{
    TimelineScrubSession session;
    QVERIFY(session.begin(MediaGeneration{3}, 0.4));
    QVERIFY(session.update(0.6));
    QVERIFY(session.cancel());
    QVERIFY(session.phase() == TimelineScrubPhase::Idle);
    QVERIFY(!session.cancel());
    QVERIFY(!session.acknowledgePending());
}

void TimelineScrubSessionTest::generationChangeCancelsInteraction()
{
    TimelineScrubSession session;
    QVERIFY(session.begin(MediaGeneration{4}, 0.3));
    QVERIFY(!session.cancelIfGenerationChanged(MediaGeneration{4}));
    QVERIFY(session.isScrubbing());
    QVERIFY(session.cancelIfGenerationChanged(MediaGeneration{5}));
    QVERIFY(!session.isActive());

    QVERIFY(session.begin(MediaGeneration{6}, 0.5));
    QVERIFY(session.commit(0.7).has_value());
    QVERIFY(session.isPendingCommit());
    QVERIFY(session.cancelIfGenerationChanged(MediaGeneration{7}));
    QVERIFY(!session.isActive());
}

void TimelineScrubSessionTest::invalidInputIsRejectedAndFiniteValuesClamp()
{
    TimelineScrubSession session;
    QVERIFY(!session.begin(MediaGeneration{}, 0.2));
    QVERIFY(!session.begin(
        MediaGeneration{1},
        std::numeric_limits<double>::quiet_NaN()));

    QVERIFY(session.begin(MediaGeneration{1}, -0.5));
    QVERIFY(fuzzyEqual(session.previewNormalized(), 0.0));
    QVERIFY(session.update(1.5));
    QVERIFY(fuzzyEqual(session.previewNormalized(), 1.0));

    QVERIFY(!session.update(std::numeric_limits<double>::infinity()));
    QVERIFY(!session.commit(std::numeric_limits<double>::quiet_NaN()).has_value());
    QVERIFY(session.isScrubbing());
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::TimelineScrubSessionTest)
#include "timeline_scrub_session_test.moc"
