#pragma once

#include "foundation/ids/request_id.h"
#include "playback/domain/state/media_generation.h"

#include <QtGlobal>

#include <chrono>
#include <cstddef>
#include <optional>

namespace player::playback::application {

using PlaybackRequestClock = std::chrono::steady_clock;

enum class PlaybackRequestType : quint8
{
    LoadMedia,
    Play,
    Pause,
    Stop,
    SeekAbsolute,
    SeekRelative,
    SetVolume,
    SetMuted,
    SetSpeed,
    SelectAudioTrack,
    SelectSubtitleTrack,
    SelectVideoTrack,
};

enum class PlaybackRequestState : quint8
{
    Pending,
    Completed,
    Cancelled,
};

enum class PlaybackRequestCancellationReason : quint8
{
    None,
    Superseded,
    GenerationChanged,
    SubmissionFailed,
    Timeout,
    Shutdown,
};

struct PlaybackRequestRecord final
{
    player::ids::RequestId requestId;
    PlaybackRequestType type = PlaybackRequestType::LoadMedia;
    std::optional<player::playback::domain::MediaGeneration> generation;
    PlaybackRequestClock::time_point submittedAt;
    PlaybackRequestState state = PlaybackRequestState::Pending;
    std::optional<bool> succeeded;
    PlaybackRequestCancellationReason cancellationReason = PlaybackRequestCancellationReason::None;
};

enum class RequestTrackStatus : quint8
{
    Tracked,
    InvalidRequestId,
    DuplicateRequestId,
    NotTrackable,
};

enum class RequestReplyDisposition : quint8
{
    Completed,
    Cancelled,
    Unknown,
    Duplicate,
    StaleGeneration,
};

struct RequestReplyResolution final
{
    RequestReplyDisposition disposition = RequestReplyDisposition::Unknown;
    std::optional<PlaybackRequestRecord> record;

    [[nodiscard]] bool accepted() const noexcept
    {
        return disposition == RequestReplyDisposition::Completed;
    }
};

struct RequestTrackerDiagnostics final
{
    std::size_t unknownReplyCount = 0;
    std::size_t duplicateReplyCount = 0;
    std::size_t cancelledReplyCount = 0;
    std::size_t staleGenerationReplyCount = 0;
    std::size_t timeoutCancellationCount = 0;
    std::size_t supersessionCancellationCount = 0;
};

} // namespace player::playback::application
