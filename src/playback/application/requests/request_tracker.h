#pragma once

#include "playback/application/requests/playback_request.h"
#include "playback/domain/commands/playback_command.h"
#include "playback/domain/events/command_event.h"

#include <chrono>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

namespace player::playback::application {

class RequestTracker final
{
public:
    [[nodiscard]] RequestTrackStatus track(
        const player::playback::domain::PlaybackCommand& command,
        player::playback::domain::MediaGeneration generation = {},
        PlaybackRequestClock::time_point submittedAt = PlaybackRequestClock::now());

    [[nodiscard]] std::size_t supersedePendingFor(
        const player::playback::domain::PlaybackCommand& replacement,
        player::playback::domain::MediaGeneration generation) noexcept;

    [[nodiscard]] RequestReplyResolution resolve(
        const player::playback::domain::CommandReplyEvent& reply,
        player::playback::domain::MediaGeneration currentGeneration);

    [[nodiscard]] bool cancel(
        player::ids::RequestId requestId,
        PlaybackRequestCancellationReason reason) noexcept;

    [[nodiscard]] std::size_t cancelMediaRequestsForGenerationChange(
        player::playback::domain::MediaGeneration nextGeneration) noexcept;

    [[nodiscard]] std::vector<PlaybackRequestRecord> cancelExpiredRecords(
        PlaybackRequestClock::time_point now,
        std::chrono::milliseconds timeout);

    [[nodiscard]] std::size_t cancelExpired(
        PlaybackRequestClock::time_point now,
        std::chrono::milliseconds timeout) noexcept;

    [[nodiscard]] std::size_t cancelAll(
        PlaybackRequestCancellationReason reason) noexcept;

    [[nodiscard]] std::optional<PlaybackRequestRecord> record(
        player::ids::RequestId requestId) const;
    [[nodiscard]] std::size_t pendingCount() const noexcept;
    [[nodiscard]] const RequestTrackerDiagnostics& diagnostics() const noexcept;

private:
    [[nodiscard]] static std::optional<PlaybackRequestType> requestTypeFor(
        const player::playback::domain::PlaybackCommand& command) noexcept;
    [[nodiscard]] static bool isMediaScoped(PlaybackRequestType type) noexcept;

    static void markCancelled(
        PlaybackRequestRecord& record,
        PlaybackRequestCancellationReason reason) noexcept;

    std::unordered_map<quint64, PlaybackRequestRecord> records_;
    RequestTrackerDiagnostics diagnostics_;
};

} // namespace player::playback::application
