#include "playback/application/requests/request_supersession_policy.h"
#include "playback/application/requests/request_tracker.h"

#include "foundation/ids/request_id.h"
#include "playback/domain/commands/load_media_command.h"
#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/transport_command.h"
#include "playback/domain/commands/volume_command.h"

#include <QtTest/QTest>

#include <utility>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;

PlaybackCommand makeCommand(quint64 requestId, PlaybackCommandPayload payload)
{
    return PlaybackCommand{player::ids::RequestId{requestId}, std::move(payload)};
}

CommandReplyEvent makeReply(quint64 requestId, bool succeeded)
{
    return CommandReplyEvent{player::ids::RequestId{requestId}, succeeded, std::nullopt};
}

PlaybackRequestRecord makePendingRecord(
    quint64 requestId,
    PlaybackRequestType type,
    MediaGeneration generation)
{
    PlaybackRequestRecord record;
    record.requestId = player::ids::RequestId{requestId};
    record.type = type;
    record.generation = generation;
    return record;
}

} // namespace

class RequestSupersessionTest final : public QObject
{
    Q_OBJECT

private slots:
    void classifiesSupersessionGroups();
    void loadReplacementCancelsOlderLoadAcrossGenerations();
    void seekReplacementIsLatestWinsWithinGeneration();
    void trackSelectionPolicyUsesIndependentLanes();
    void delayPolicyUsesIndependentLanes();
    void shutdownCancelsAllRemainingPendingRequests();
};

void RequestSupersessionTest::classifiesSupersessionGroups()
{
    const auto load = requestSupersessionGroupFor(PlaybackRequestType::LoadMedia);
    QVERIFY(load.has_value());
    QVERIFY(*load == PlaybackRequestSupersessionGroup::LoadMedia);

    const auto absoluteSeek = requestSupersessionGroupFor(PlaybackRequestType::SeekAbsolute);
    const auto relativeSeek = requestSupersessionGroupFor(PlaybackRequestType::SeekRelative);
    QVERIFY(absoluteSeek.has_value());
    QVERIFY(relativeSeek.has_value());
    QVERIFY(*absoluteSeek == PlaybackRequestSupersessionGroup::Seek);
    QVERIFY(*relativeSeek == PlaybackRequestSupersessionGroup::Seek);

    const auto audio = requestSupersessionGroupFor(PlaybackRequestType::SelectAudioTrack);
    const auto subtitle = requestSupersessionGroupFor(PlaybackRequestType::SelectSubtitleTrack);
    const auto video = requestSupersessionGroupFor(PlaybackRequestType::SelectVideoTrack);
    QVERIFY(audio.has_value());
    QVERIFY(subtitle.has_value());
    QVERIFY(video.has_value());
    QVERIFY(*audio == PlaybackRequestSupersessionGroup::AudioTrackSelection);
    QVERIFY(*subtitle == PlaybackRequestSupersessionGroup::SubtitleTrackSelection);
    QVERIFY(*video == PlaybackRequestSupersessionGroup::VideoTrackSelection);

    const auto subtitleDelay =
        requestSupersessionGroupFor(PlaybackRequestType::SetSubtitleDelay);
    const auto audioDelay = requestSupersessionGroupFor(PlaybackRequestType::SetAudioDelay);
    QVERIFY(subtitleDelay.has_value());
    QVERIFY(audioDelay.has_value());
    QVERIFY(*subtitleDelay == PlaybackRequestSupersessionGroup::SubtitleDelay);
    QVERIFY(*audioDelay == PlaybackRequestSupersessionGroup::AudioDelay);
    QVERIFY(!requestSupersessionGroupFor(PlaybackRequestType::LoadExternalSubtitle).has_value());

    QVERIFY(!requestSupersessionGroupFor(PlaybackRequestType::Play).has_value());
    QVERIFY(!requestSupersessionGroupFor(PlaybackRequestType::Pause).has_value());
    QVERIFY(!requestSupersessionGroupFor(PlaybackRequestType::Stop).has_value());
    QVERIFY(!requestSupersessionGroupFor(PlaybackRequestType::SetVolume).has_value());
    QVERIFY(!requestSupersessionGroupFor(PlaybackRequestType::SetMuted).has_value());
    QVERIFY(!requestSupersessionGroupFor(PlaybackRequestType::SetSpeed).has_value());
}

void RequestSupersessionTest::loadReplacementCancelsOlderLoadAcrossGenerations()
{
    RequestTracker tracker;
    const MediaGeneration generationA{41};
    const MediaGeneration generationB{42};

    const PlaybackCommand loadA = makeCommand(
        1,
        LoadMediaCommand{QStringLiteral("A.wav")});
    const PlaybackCommand volume = makeCommand(2, SetVolumeCommand{60.0});
    const PlaybackCommand loadB = makeCommand(
        3,
        LoadMediaCommand{QStringLiteral("B.wav")});

    QVERIFY(tracker.track(loadA, generationA) == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(volume, generationA) == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(loadB, generationB) == RequestTrackStatus::Tracked);

    QCOMPARE(tracker.supersedePendingFor(loadB, generationB), std::size_t{1});
    QCOMPARE(tracker.diagnostics().supersessionCancellationCount, std::size_t{1});
    QCOMPARE(tracker.cancelMediaRequestsForGenerationChange(generationB), std::size_t{0});

    const auto oldLoad = tracker.record(player::ids::RequestId{1});
    QVERIFY(oldLoad.has_value());
    QVERIFY(oldLoad->state == PlaybackRequestState::Cancelled);
    QVERIFY(oldLoad->cancellationReason == PlaybackRequestCancellationReason::Superseded);

    const auto sessionControl = tracker.record(player::ids::RequestId{2});
    QVERIFY(sessionControl.has_value());
    QVERIFY(sessionControl->state == PlaybackRequestState::Pending);

    const auto replacement = tracker.record(player::ids::RequestId{3});
    QVERIFY(replacement.has_value());
    QVERIFY(replacement->state == PlaybackRequestState::Pending);

    const RequestReplyResolution oldReply = tracker.resolve(makeReply(1, false), generationB);
    QVERIFY(!oldReply.accepted());
    QVERIFY(oldReply.disposition == RequestReplyDisposition::Cancelled);
    QVERIFY(oldReply.record.has_value());
    QVERIFY(oldReply.record->cancellationReason == PlaybackRequestCancellationReason::Superseded);

    const RequestReplyResolution newReply = tracker.resolve(makeReply(3, true), generationB);
    QVERIFY(newReply.accepted());
    QVERIFY(newReply.disposition == RequestReplyDisposition::Completed);
}

void RequestSupersessionTest::seekReplacementIsLatestWinsWithinGeneration()
{
    RequestTracker tracker;
    const MediaGeneration generation{7};
    const MediaGeneration otherGeneration{8};

    const PlaybackCommand seek1 = makeCommand(10, SeekCommand{2.0, SeekMode::Absolute});
    const PlaybackCommand seek2 = makeCommand(11, SeekCommand{1.0, SeekMode::Relative});
    const PlaybackCommand otherGenerationSeek = makeCommand(
        12,
        SeekCommand{4.0, SeekMode::Absolute});

    QVERIFY(tracker.track(seek1, generation) == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(otherGenerationSeek, otherGeneration) == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(seek2, generation) == RequestTrackStatus::Tracked);

    QCOMPARE(tracker.supersedePendingFor(seek2, generation), std::size_t{1});

    const auto oldSeek = tracker.record(player::ids::RequestId{10});
    QVERIFY(oldSeek.has_value());
    QVERIFY(oldSeek->state == PlaybackRequestState::Cancelled);
    QVERIFY(oldSeek->cancellationReason == PlaybackRequestCancellationReason::Superseded);

    const auto otherSeek = tracker.record(player::ids::RequestId{12});
    QVERIFY(otherSeek.has_value());
    QVERIFY(otherSeek->state == PlaybackRequestState::Pending);

    const RequestReplyResolution oldReply = tracker.resolve(makeReply(10, false), generation);
    QVERIFY(!oldReply.accepted());
    QVERIFY(oldReply.disposition == RequestReplyDisposition::Cancelled);

    const RequestReplyResolution latestReply = tracker.resolve(makeReply(11, true), generation);
    QVERIFY(latestReply.accepted());
    QVERIFY(latestReply.disposition == RequestReplyDisposition::Completed);
}

void RequestSupersessionTest::trackSelectionPolicyUsesIndependentLanes()
{
    const MediaGeneration generation{55};
    const MediaGeneration nextGeneration{56};

    const PlaybackRequestRecord audio = makePendingRecord(
        20,
        PlaybackRequestType::SelectAudioTrack,
        generation);
    const PlaybackRequestRecord subtitle = makePendingRecord(
        21,
        PlaybackRequestType::SelectSubtitleTrack,
        generation);
    const PlaybackRequestRecord video = makePendingRecord(
        22,
        PlaybackRequestType::SelectVideoTrack,
        generation);

    QVERIFY(shouldSupersedeRequest(
        audio,
        PlaybackRequestType::SelectAudioTrack,
        generation));
    QVERIFY(!shouldSupersedeRequest(
        audio,
        PlaybackRequestType::SelectSubtitleTrack,
        generation));
    QVERIFY(!shouldSupersedeRequest(
        audio,
        PlaybackRequestType::SelectAudioTrack,
        nextGeneration));

    QVERIFY(shouldSupersedeRequest(
        subtitle,
        PlaybackRequestType::SelectSubtitleTrack,
        generation));
    QVERIFY(!shouldSupersedeRequest(
        subtitle,
        PlaybackRequestType::SelectVideoTrack,
        generation));

    QVERIFY(shouldSupersedeRequest(
        video,
        PlaybackRequestType::SelectVideoTrack,
        generation));
    QVERIFY(!shouldSupersedeRequest(
        video,
        PlaybackRequestType::SelectAudioTrack,
        generation));

    PlaybackRequestRecord completedAudio = audio;
    completedAudio.state = PlaybackRequestState::Completed;
    QVERIFY(!shouldSupersedeRequest(
        completedAudio,
        PlaybackRequestType::SelectAudioTrack,
        generation));
}

void RequestSupersessionTest::delayPolicyUsesIndependentLanes()
{
    const MediaGeneration generation{57};
    const MediaGeneration nextGeneration{58};

    const PlaybackRequestRecord subtitleDelay = makePendingRecord(
        23,
        PlaybackRequestType::SetSubtitleDelay,
        generation);
    const PlaybackRequestRecord audioDelay = makePendingRecord(
        24,
        PlaybackRequestType::SetAudioDelay,
        generation);

    QVERIFY(shouldSupersedeRequest(
        subtitleDelay,
        PlaybackRequestType::SetSubtitleDelay,
        generation));
    QVERIFY(!shouldSupersedeRequest(
        subtitleDelay,
        PlaybackRequestType::SetAudioDelay,
        generation));
    QVERIFY(!shouldSupersedeRequest(
        subtitleDelay,
        PlaybackRequestType::SetSubtitleDelay,
        nextGeneration));

    QVERIFY(shouldSupersedeRequest(
        audioDelay,
        PlaybackRequestType::SetAudioDelay,
        generation));
    QVERIFY(!shouldSupersedeRequest(
        audioDelay,
        PlaybackRequestType::SetSubtitleDelay,
        generation));
}

void RequestSupersessionTest::shutdownCancelsAllRemainingPendingRequests()
{
    RequestTracker tracker;
    const MediaGeneration generation{90};

    const PlaybackCommand seek = makeCommand(30, SeekCommand{3.0, SeekMode::Absolute});
    const PlaybackCommand volume = makeCommand(31, SetVolumeCommand{45.0});
    const PlaybackCommand play = makeCommand(32, TransportCommand{TransportAction::Play});

    QVERIFY(tracker.track(seek, generation) == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(volume, generation) == RequestTrackStatus::Tracked);
    QVERIFY(tracker.track(play, generation) == RequestTrackStatus::Tracked);

    QCOMPARE(
        tracker.cancelAll(PlaybackRequestCancellationReason::Shutdown),
        std::size_t{3});
    QCOMPARE(tracker.pendingCount(), std::size_t{0});

    for (const quint64 id : {quint64{30}, quint64{31}, quint64{32}}) {
        const auto record = tracker.record(player::ids::RequestId{id});
        QVERIFY(record.has_value());
        QVERIFY(record->state == PlaybackRequestState::Cancelled);
        QVERIFY(record->cancellationReason == PlaybackRequestCancellationReason::Shutdown);

        const RequestReplyResolution lateReply = tracker.resolve(makeReply(id, false), generation);
        QVERIFY(!lateReply.accepted());
        QVERIFY(lateReply.disposition == RequestReplyDisposition::Cancelled);
    }
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::RequestSupersessionTest)
#include "request_supersession_test.moc"
