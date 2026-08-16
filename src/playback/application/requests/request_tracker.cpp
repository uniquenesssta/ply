#include "request_tracker.h"

#include "request_supersession_policy.h"
#include "playback/domain/commands/load_media_command.h"
#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/speed_command.h"
#include "playback/domain/commands/transport_command.h"
#include "playback/domain/commands/volume_command.h"

#include <algorithm>
#include <utility>
#include <variant>

namespace player::playback::application {

using namespace player::playback::domain;

RequestTrackStatus RequestTracker::track(
    const PlaybackCommand& command,
    MediaGeneration generation,
    PlaybackRequestClock::time_point submittedAt)
{
    const player::ids::RequestId requestId = command.requestId();
    if (!requestId.isValid()) {
        return RequestTrackStatus::InvalidRequestId;
    }

    if (records_.contains(requestId.value())) {
        return RequestTrackStatus::DuplicateRequestId;
    }

    const std::optional<PlaybackRequestType> type = requestTypeFor(command);
    if (!type.has_value()) {
        return RequestTrackStatus::NotTrackable;
    }

    PlaybackRequestRecord record;
    record.requestId = requestId;
    record.type = *type;
    record.submittedAt = submittedAt;
    if (isMediaScoped(*type)) {
        record.generation = generation;
    }

    records_.emplace(requestId.value(), std::move(record));
    return RequestTrackStatus::Tracked;
}

std::size_t RequestTracker::supersedePendingFor(
    const PlaybackCommand& replacement,
    MediaGeneration generation) noexcept
{
    const std::optional<PlaybackRequestType> replacementType = requestTypeFor(replacement);
    if (!replacementType.has_value()) {
        return 0;
    }

    std::size_t cancelled = 0;
    for (auto& [id, record] : records_) {
        Q_UNUSED(id);
        if (record.requestId == replacement.requestId()) {
            continue;
        }
        if (!shouldSupersedeRequest(record, *replacementType, generation)) {
            continue;
        }

        markCancelled(record, PlaybackRequestCancellationReason::Superseded);
        ++cancelled;
    }

    diagnostics_.supersessionCancellationCount += cancelled;
    return cancelled;
}

RequestReplyResolution RequestTracker::resolve(
    const CommandReplyEvent& reply,
    MediaGeneration currentGeneration)
{
    const auto it = records_.find(reply.requestId.value());
    if (it == records_.end()) {
        ++diagnostics_.unknownReplyCount;
        return RequestReplyResolution{RequestReplyDisposition::Unknown, std::nullopt};
    }

    PlaybackRequestRecord& record = it->second;
    if (record.state == PlaybackRequestState::Completed) {
        ++diagnostics_.duplicateReplyCount;
        return RequestReplyResolution{RequestReplyDisposition::Duplicate, record};
    }
    if (record.state == PlaybackRequestState::Cancelled) {
        ++diagnostics_.cancelledReplyCount;
        return RequestReplyResolution{RequestReplyDisposition::Cancelled, record};
    }

    if (record.generation.has_value() && *record.generation != currentGeneration) {
        markCancelled(record, PlaybackRequestCancellationReason::GenerationChanged);
        ++diagnostics_.staleGenerationReplyCount;
        return RequestReplyResolution{RequestReplyDisposition::StaleGeneration, record};
    }

    record.state = PlaybackRequestState::Completed;
    record.succeeded = reply.succeeded;
    record.cancellationReason = PlaybackRequestCancellationReason::None;
    return RequestReplyResolution{RequestReplyDisposition::Completed, record};
}

bool RequestTracker::cancel(
    player::ids::RequestId requestId,
    PlaybackRequestCancellationReason reason) noexcept
{
    const auto it = records_.find(requestId.value());
    if (it == records_.end() || it->second.state != PlaybackRequestState::Pending) {
        return false;
    }

    markCancelled(it->second, reason);
    return true;
}

std::size_t RequestTracker::cancelMediaRequestsForGenerationChange(
    MediaGeneration nextGeneration) noexcept
{
    std::size_t cancelled = 0;
    for (auto& [id, record] : records_) {
        Q_UNUSED(id);
        if (record.state != PlaybackRequestState::Pending || !record.generation.has_value()) {
            continue;
        }
        if (*record.generation == nextGeneration) {
            continue;
        }

        markCancelled(record, PlaybackRequestCancellationReason::GenerationChanged);
        ++cancelled;
    }
    return cancelled;
}

std::vector<PlaybackRequestRecord> RequestTracker::cancelExpiredRecords(
    PlaybackRequestClock::time_point now,
    std::chrono::milliseconds timeout) noexcept
{
    std::vector<PlaybackRequestRecord> cancelledRecords;
    if (timeout.count() < 0) {
        return cancelledRecords;
    }

    for (auto& [id, record] : records_) {
        Q_UNUSED(id);
        if (record.state != PlaybackRequestState::Pending || now < record.submittedAt) {
            continue;
        }
        if (now - record.submittedAt < timeout) {
            continue;
        }

        markCancelled(record, PlaybackRequestCancellationReason::Timeout);
        cancelledRecords.push_back(record);
    }

    diagnostics_.timeoutCancellationCount += cancelledRecords.size();
    return cancelledRecords;
}

std::size_t RequestTracker::cancelExpired(
    PlaybackRequestClock::time_point now,
    std::chrono::milliseconds timeout) noexcept
{
    return cancelExpiredRecords(now, timeout).size();
}

std::size_t RequestTracker::cancelAll(PlaybackRequestCancellationReason reason) noexcept
{
    std::size_t cancelled = 0;
    for (auto& [id, record] : records_) {
        Q_UNUSED(id);
        if (record.state != PlaybackRequestState::Pending) {
            continue;
        }

        markCancelled(record, reason);
        ++cancelled;
    }
    return cancelled;
}

std::optional<PlaybackRequestRecord> RequestTracker::record(
    player::ids::RequestId requestId) const
{
    const auto it = records_.find(requestId.value());
    if (it == records_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::size_t RequestTracker::pendingCount() const noexcept
{
    return static_cast<std::size_t>(std::count_if(
        records_.cbegin(),
        records_.cend(),
        [](const auto& item) {
            return item.second.state == PlaybackRequestState::Pending;
        }));
}

const RequestTrackerDiagnostics& RequestTracker::diagnostics() const noexcept
{
    return diagnostics_;
}

std::optional<PlaybackRequestType> RequestTracker::requestTypeFor(
    const PlaybackCommand& command) noexcept
{
    const PlaybackCommandPayload& payload = command.payload();
    if (std::holds_alternative<LoadMediaCommand>(payload)) {
        return PlaybackRequestType::LoadMedia;
    }

    if (const auto* transport = std::get_if<TransportCommand>(&payload)) {
        switch (transport->action) {
        case TransportAction::Play:
            return PlaybackRequestType::Play;
        case TransportAction::Pause:
            return PlaybackRequestType::Pause;
        case TransportAction::Stop:
            return PlaybackRequestType::Stop;
        }
    }

    if (const auto* seek = std::get_if<SeekCommand>(&payload)) {
        return seek->mode == SeekMode::Absolute
            ? PlaybackRequestType::SeekAbsolute
            : PlaybackRequestType::SeekRelative;
    }
    if (std::holds_alternative<SetVolumeCommand>(payload)) {
        return PlaybackRequestType::SetVolume;
    }
    if (std::holds_alternative<SetMutedCommand>(payload)) {
        return PlaybackRequestType::SetMuted;
    }
    if (std::holds_alternative<SetSpeedCommand>(payload)) {
        return PlaybackRequestType::SetSpeed;
    }

    return std::nullopt;
}

bool RequestTracker::isMediaScoped(PlaybackRequestType type) noexcept
{
    switch (type) {
    case PlaybackRequestType::LoadMedia:
    case PlaybackRequestType::Play:
    case PlaybackRequestType::Pause:
    case PlaybackRequestType::Stop:
    case PlaybackRequestType::SeekAbsolute:
    case PlaybackRequestType::SeekRelative:
    case PlaybackRequestType::SelectAudioTrack:
    case PlaybackRequestType::SelectSubtitleTrack:
    case PlaybackRequestType::SelectVideoTrack:
        return true;
    case PlaybackRequestType::SetVolume:
    case PlaybackRequestType::SetMuted:
    case PlaybackRequestType::SetSpeed:
        return false;
    }
    return false;
}

void RequestTracker::markCancelled(
    PlaybackRequestRecord& record,
    PlaybackRequestCancellationReason reason) noexcept
{
    record.state = PlaybackRequestState::Cancelled;
    record.succeeded.reset();
    record.cancellationReason = reason;
}

} // namespace player::playback::application
