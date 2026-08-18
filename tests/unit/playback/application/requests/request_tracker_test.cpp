#include "playback/application/requests/request_tracker.h"

#include "foundation/ids/request_id.h"
#include "playback/domain/commands/lifecycle_command.h"
#include "playback/domain/commands/load_media_command.h"
#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/track_selection_command.h"
#include "playback/domain/commands/transport_command.h"
#include "playback/domain/commands/volume_command.h"

#include <QtTest/QTest>

#include <chrono>
#include <utility>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;
using namespace std::chrono_literals;

PlaybackCommand makeCommand(quint64 requestId, PlaybackCommandPayload payload)
{
    return PlaybackCommand{player::ids::RequestId{requestId}, std::move(payload)};
}

CommandReplyEvent makeReply(quint64 requestId, bool succeeded = true)
{
    return CommandReplyEvent{player::ids::RequestId{requestId}, succeeded, std::nullopt};
}

} // namespace

class RequestTrackerTest final : public QObject
{
    Q_OBJECT

private slots:
    void tracksTypeGenerationAndSubmissionTime();
    void rejectsDuplicateAndLifecycleCommands();
    void generationChangeCancelsOnlyOldMediaRequests();
    void trackSelectionUsesGenerationAndSameKindSupersession();
    void replyCompletesOnlyOnce();
    void staleCancelledAndUnknownRepliesAreIgnored();
    void timeoutAndShutdownCancelPending();
};

void RequestTrackerTest::tracksTypeGenerationAndSubmissionTime()
{
    RequestTracker tracker;
    const auto submittedAt = PlaybackRequestClock::time_point{} + 42s;

    QVERIFY(tracker.track(
                makeCommand(1, SeekCommand{3.0, SeekMode::Relative}),
                MediaGeneration{7},
                submittedAt)
        == RequestTrackStatus::Tracked);

    const auto record = tracker.record(player::ids::RequestId{1});
    QVERIFY(record.has_value());
    QVERIFY(record->type == PlaybackRequestType::SeekRelative);
    QVERIFY(record->generation.has_value());
    QCOMPARE(record->generation->value(), quint64{7});
    QVERIFY(record->submittedAt == submittedAt);
    QVERIFY(record->state == PlaybackRequestState::Pending);
    QVERIFY(!record->succeeded.has_value());
    QCOMPARE(tracker.pendingCount(), std::size_t{1});

    QVERIFY(tracker.track(
                makeCommand(2, SetVolumeCommand{65.0}),
                MediaGeneration{7},
                submittedAt)
        == RequestTrackStatus::Tracked);
    const auto controlRecord = tracker.record(player::ids::RequestId{2});
    QVERIFY(controlRecord.has_value());
    QVERIFY(controlRecord->type == PlaybackRequestType::SetVolume);
    QVERIFY(!controlRecord->generation.has_value());
}

void RequestTrackerTest::rejectsDuplicateAndLifecycleCommands()
{
    RequestTracker tracker;

    QVERIFY(tracker.track(
                makeCommand(3, TransportCommand{TransportAction::Play}),
                MediaGeneration{1})
        == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(
                makeCommand(3, TransportCommand{TransportAction::Pause}),
                MediaGeneration{1})
        == RequestTrackStatus::DuplicateRequestId);
    QVERIFY(tracker.track(
                makeCommand(4, LifecycleCommand{PlaybackLifecycleAction::Initialize}))
        == RequestTrackStatus::NotTrackable);
    QVERIFY(tracker.track(
                makeCommand(0, TransportCommand{TransportAction::Play}),
                MediaGeneration{1})
        == RequestTrackStatus::InvalidRequestId);
    QCOMPARE(tracker.pendingCount(), std::size_t{1});
}

void RequestTrackerTest::generationChangeCancelsOnlyOldMediaRequests()
{
    RequestTracker tracker;
    const MediaGeneration generation1{11};
    const MediaGeneration generation2{12};

    QVERIFY(tracker.track(
                makeCommand(9, TransportCommand{TransportAction::Play}),
                MediaGeneration{})
        == RequestTrackStatus::Tracked);
    const auto noMediaRecord = tracker.record(player::ids::RequestId{9});
    QVERIFY(noMediaRecord.has_value());
    QVERIFY(noMediaRecord->generation.has_value());
    QVERIFY(!noMediaRecord->generation->isValid());

    QVERIFY(tracker.track(
                makeCommand(10, LoadMediaCommand{QStringLiteral("A.wav")}),
                generation1)
        == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(
                makeCommand(11, TransportCommand{TransportAction::Pause}),
                generation1)
        == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(
                makeCommand(12, SeekCommand{1.0, SeekMode::Absolute}),
                generation1)
        == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(
                makeCommand(13, SetVolumeCommand{55.0}),
                generation1)
        == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(
                makeCommand(14, LoadMediaCommand{QStringLiteral("B.wav")}),
                generation2)
        == RequestTrackStatus::Tracked);

    QCOMPARE(tracker.cancelMediaRequestsForGenerationChange(generation2), std::size_t{4});
    QCOMPARE(tracker.pendingCount(), std::size_t{2});

    for (const quint64 id : {quint64{9}, quint64{10}, quint64{11}, quint64{12}}) {
        const auto record = tracker.record(player::ids::RequestId{id});
        QVERIFY(record.has_value());
        QVERIFY(record->state == PlaybackRequestState::Cancelled);
        QVERIFY(record->cancellationReason == PlaybackRequestCancellationReason::GenerationChanged);
    }

    const auto controlRecord = tracker.record(player::ids::RequestId{13});
    QVERIFY(controlRecord.has_value());
    QVERIFY(controlRecord->state == PlaybackRequestState::Pending);
    QVERIFY(controlRecord->cancellationReason == PlaybackRequestCancellationReason::None);

    const auto replacementRecord = tracker.record(player::ids::RequestId{14});
    QVERIFY(replacementRecord.has_value());
    QVERIFY(replacementRecord->state == PlaybackRequestState::Pending);
    QVERIFY(replacementRecord->generation.has_value());
    QCOMPARE(replacementRecord->generation->value(), generation2.value());

    const RequestReplyResolution oldReply = tracker.resolve(makeReply(10, false), generation2);
    QVERIFY(!oldReply.accepted());
    QVERIFY(oldReply.disposition == RequestReplyDisposition::Cancelled);

    const RequestReplyResolution currentReply = tracker.resolve(makeReply(14, true), generation2);
    QVERIFY(currentReply.accepted());
    QVERIFY(currentReply.record.has_value());
    QVERIFY(currentReply.record->state == PlaybackRequestState::Completed);
}

void RequestTrackerTest::trackSelectionUsesGenerationAndSameKindSupersession()
{
    RequestTracker tracker;
    const MediaGeneration generation{20};

    const PlaybackCommand firstAudio = makeCommand(
        50,
        TrackSelectionCommand{TrackSelectionKind::Audio, qint64{2}});
    QVERIFY(tracker.track(firstAudio, generation) == RequestTrackStatus::Tracked);
    const auto firstAudioRecord = tracker.record(player::ids::RequestId{50});
    QVERIFY(firstAudioRecord.has_value());
    QVERIFY(firstAudioRecord->type == PlaybackRequestType::SelectAudioTrack);
    QVERIFY(firstAudioRecord->generation.has_value());
    QCOMPARE(firstAudioRecord->generation->value(), generation.value());

    const PlaybackCommand secondAudio = makeCommand(
        51,
        TrackSelectionCommand{TrackSelectionKind::Audio, qint64{3}});
    QVERIFY(tracker.track(secondAudio, generation) == RequestTrackStatus::Tracked);
    QCOMPARE(tracker.supersedePendingFor(secondAudio, generation), std::size_t{1});

    const auto supersededAudio = tracker.record(player::ids::RequestId{50});
    QVERIFY(supersededAudio.has_value());
    QVERIFY(supersededAudio->state == PlaybackRequestState::Cancelled);
    QVERIFY(supersededAudio->cancellationReason == PlaybackRequestCancellationReason::Superseded);

    const auto currentAudio = tracker.record(player::ids::RequestId{51});
    QVERIFY(currentAudio.has_value());
    QVERIFY(currentAudio->state == PlaybackRequestState::Pending);

    const PlaybackCommand subtitle = makeCommand(
        52,
        TrackSelectionCommand{TrackSelectionKind::Subtitle, qint64{7}});
    QVERIFY(tracker.track(subtitle, generation) == RequestTrackStatus::Tracked);
    QCOMPARE(tracker.supersedePendingFor(subtitle, generation), std::size_t{0});

    const auto subtitleRecord = tracker.record(player::ids::RequestId{52});
    QVERIFY(subtitleRecord.has_value());
    QVERIFY(subtitleRecord->type == PlaybackRequestType::SelectSubtitleTrack);
    QVERIFY(subtitleRecord->state == PlaybackRequestState::Pending);

    const auto stillPendingAudio = tracker.record(player::ids::RequestId{51});
    QVERIFY(stillPendingAudio.has_value());
    QVERIFY(stillPendingAudio->state == PlaybackRequestState::Pending);
}

void RequestTrackerTest::replyCompletesOnlyOnce()
{
    RequestTracker tracker;
    const MediaGeneration generation{21};

    QVERIFY(tracker.track(
                makeCommand(20, SeekCommand{2.0, SeekMode::Absolute}),
                generation)
        == RequestTrackStatus::Tracked);

    const RequestReplyResolution first = tracker.resolve(makeReply(20, true), generation);
    QVERIFY(first.accepted());
    QVERIFY(first.record.has_value());
    QVERIFY(first.record->state == PlaybackRequestState::Completed);
    QVERIFY(first.record->succeeded.has_value());
    QVERIFY(*first.record->succeeded);
    QCOMPARE(tracker.pendingCount(), std::size_t{0});

    const RequestReplyResolution duplicate = tracker.resolve(makeReply(20, true), generation);
    QVERIFY(!duplicate.accepted());
    QVERIFY(duplicate.disposition == RequestReplyDisposition::Duplicate);
    QCOMPARE(tracker.diagnostics().duplicateReplyCount, std::size_t{1});
}

void RequestTrackerTest::staleCancelledAndUnknownRepliesAreIgnored()
{
    RequestTracker tracker;

    QVERIFY(tracker.track(
                makeCommand(30, LoadMediaCommand{QStringLiteral("A.wav")}),
                MediaGeneration{31})
        == RequestTrackStatus::Tracked);

    const RequestReplyResolution stale = tracker.resolve(makeReply(30, false), MediaGeneration{32});
    QVERIFY(!stale.accepted());
    QVERIFY(stale.disposition == RequestReplyDisposition::StaleGeneration);
    QVERIFY(stale.record.has_value());
    QVERIFY(stale.record->state == PlaybackRequestState::Cancelled);
    QVERIFY(stale.record->cancellationReason == PlaybackRequestCancellationReason::GenerationChanged);
    QCOMPARE(tracker.diagnostics().staleGenerationReplyCount, std::size_t{1});

    QVERIFY(tracker.track(
                makeCommand(31, TransportCommand{TransportAction::Pause}),
                MediaGeneration{32})
        == RequestTrackStatus::Tracked);
    QVERIFY(tracker.cancel(
        player::ids::RequestId{31},
        PlaybackRequestCancellationReason::SubmissionFailed));

    const RequestReplyResolution cancelled = tracker.resolve(makeReply(31, false), MediaGeneration{32});
    QVERIFY(!cancelled.accepted());
    QVERIFY(cancelled.disposition == RequestReplyDisposition::Cancelled);
    QCOMPARE(tracker.diagnostics().cancelledReplyCount, std::size_t{1});

    const RequestReplyResolution unknown = tracker.resolve(makeReply(999, false), MediaGeneration{32});
    QVERIFY(!unknown.accepted());
    QVERIFY(unknown.disposition == RequestReplyDisposition::Unknown);
    QCOMPARE(tracker.diagnostics().unknownReplyCount, std::size_t{1});
}

void RequestTrackerTest::timeoutAndShutdownCancelPending()
{
    RequestTracker tracker;
    const auto base = PlaybackRequestClock::time_point{} + 100s;

    QVERIFY(tracker.track(
                makeCommand(40, SetMutedCommand{true}),
                {},
                base)
        == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(
                makeCommand(41, TransportCommand{TransportAction::Play}),
                MediaGeneration{5},
                base + 20s)
        == RequestTrackStatus::Tracked);

    QCOMPARE(tracker.cancelExpired(base + 31s, 30s), std::size_t{1});
    QCOMPARE(tracker.diagnostics().timeoutCancellationCount, std::size_t{1});

    const auto timedOut = tracker.record(player::ids::RequestId{40});
    QVERIFY(timedOut.has_value());
    QVERIFY(timedOut->state == PlaybackRequestState::Cancelled);
    QVERIFY(timedOut->cancellationReason == PlaybackRequestCancellationReason::Timeout);

    QCOMPARE(
        tracker.cancelAll(PlaybackRequestCancellationReason::Shutdown),
        std::size_t{1});
    const auto shutdown = tracker.record(player::ids::RequestId{41});
    QVERIFY(shutdown.has_value());
    QVERIFY(shutdown->state == PlaybackRequestState::Cancelled);
    QVERIFY(shutdown->cancellationReason == PlaybackRequestCancellationReason::Shutdown);
    QCOMPARE(tracker.pendingCount(), std::size_t{0});
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::RequestTrackerTest)
#include "request_tracker_test.moc"
