#include "playback/application/requests/request_timeout_monitor.h"
#include "playback/application/requests/request_tracker.h"

#include "foundation/ids/request_id.h"
#include "playback/domain/commands/load_media_command.h"
#include "playback/domain/commands/playback_command.h"

#include <QtTest/QTest>

#include <chrono>
#include <optional>
#include <utility>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;
using namespace std::chrono_literals;

PlaybackCommand makeLoadCommand(quint64 requestId, QString source)
{
    return PlaybackCommand{
        player::ids::RequestId{requestId},
        LoadMediaCommand{std::move(source)}};
}

} // namespace

class RequestTimeoutRecoveryTest final : public QObject
{
    Q_OBJECT

private slots:
    void trackerReturnsTimedOutLoadMetadata();
    void monitorForwardsTimedOutLoadMetadata();
};

void RequestTimeoutRecoveryTest::trackerReturnsTimedOutLoadMetadata()
{
    RequestTracker tracker;
    const auto now = PlaybackRequestClock::now();
    const MediaGeneration generation{17};

    QVERIFY(tracker.track(
                makeLoadCommand(101, QStringLiteral("https://example.invalid/video.mp4")),
                generation,
                now - 31s)
        == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(
                makeLoadCommand(102, QStringLiteral("https://example.invalid/next.mp4")),
                MediaGeneration{18},
                now - 1s)
        == RequestTrackStatus::Tracked);

    const auto expired = tracker.cancelExpiredRecords(now, 30s);
    QCOMPARE(expired.size(), std::size_t{1});
    QCOMPARE(expired.front().requestId.value(), quint64{101});
    QVERIFY(expired.front().type == PlaybackRequestType::LoadMedia);
    QVERIFY(expired.front().generation.has_value());
    QCOMPARE(expired.front().generation->value(), generation.value());
    QVERIFY(expired.front().state == PlaybackRequestState::Cancelled);
    QVERIFY(expired.front().cancellationReason == PlaybackRequestCancellationReason::Timeout);
    QCOMPARE(tracker.pendingCount(), std::size_t{1});
    QCOMPARE(tracker.diagnostics().timeoutCancellationCount, std::size_t{1});
}

void RequestTimeoutRecoveryTest::monitorForwardsTimedOutLoadMetadata()
{
    RequestTracker tracker;
    const MediaGeneration generation{29};
    QVERIFY(tracker.track(
                makeLoadCommand(201, QStringLiteral("https://example.invalid/hanging.mp4")),
                generation,
                PlaybackRequestClock::now() - 31s)
        == RequestTrackStatus::Tracked);

    std::optional<PlaybackRequestRecord> timedOut;
    RequestTimeoutMonitor monitor(
        tracker,
        [&timedOut](const PlaybackRequestRecord& record) {
            timedOut = record;
        });

    monitor.start();
    QTRY_VERIFY_WITH_TIMEOUT(timedOut.has_value(), 2500);
    monitor.stop();

    QCOMPARE(timedOut->requestId.value(), quint64{201});
    QVERIFY(timedOut->type == PlaybackRequestType::LoadMedia);
    QVERIFY(timedOut->generation.has_value());
    QCOMPARE(timedOut->generation->value(), generation.value());
    QVERIFY(timedOut->cancellationReason == PlaybackRequestCancellationReason::Timeout);
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::RequestTimeoutRecoveryTest)
#include "request_timeout_recovery_test.moc"
