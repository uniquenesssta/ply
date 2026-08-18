#pragma once

#include "foundation/ids/request_id.h"
#include "playback/domain/commands/delay_command.h"
#include "playback/domain/commands/external_subtitle_command.h"
#include "playback/domain/commands/lifecycle_command.h"
#include "playback/domain/commands/load_media_command.h"
#include "playback/domain/commands/seek_command.h"
#include "playback/domain/commands/speed_command.h"
#include "playback/domain/commands/track_command.h"
#include "playback/domain/commands/transport_command.h"
#include "playback/domain/commands/volume_command.h"

#include <QtGlobal>

#include <optional>
#include <variant>

namespace player::playback::domain {

using PlaybackCommandPayload = std::variant<
    LoadMediaCommand,
    TransportCommand,
    SeekCommand,
    SetVolumeCommand,
    SetMutedCommand,
    SetSpeedCommand,
    SelectTrackCommand,
    SetSubtitleDelayCommand,
    SetAudioDelayCommand,
    LoadExternalSubtitleCommand,
    LifecycleCommand>;

enum class PlaybackCommandValidationError : quint8
{
    InvalidRequestId,
    EmptyMediaSource,
    MediaSourceContainsNull,
    NonFiniteSeek,
    InvalidVolume,
    InvalidSpeed,
    InvalidTrackSelection,
    NonFiniteDelay,
    InvalidExternalSubtitlePath,
};

class PlaybackCommand final
{
public:
    PlaybackCommand(player::ids::RequestId requestId, PlaybackCommandPayload payload);

    [[nodiscard]] player::ids::RequestId requestId() const noexcept;
    [[nodiscard]] const PlaybackCommandPayload& payload() const noexcept;

private:
    player::ids::RequestId requestId_;
    PlaybackCommandPayload payload_;
};

[[nodiscard]] std::optional<PlaybackCommandValidationError> validatePlaybackCommand(
    const PlaybackCommand& command) noexcept;

} // namespace player::playback::domain
