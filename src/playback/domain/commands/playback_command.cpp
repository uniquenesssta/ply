#include "playback_command.h"

#include <QByteArray>

#include <cmath>
#include <type_traits>
#include <utility>

namespace player::playback::domain {

PlaybackCommand::PlaybackCommand(
    player::ids::RequestId requestId,
    PlaybackCommandPayload payload)
    : requestId_(requestId)
    , payload_(std::move(payload))
{
}

player::ids::RequestId PlaybackCommand::requestId() const noexcept
{
    return requestId_;
}

const PlaybackCommandPayload& PlaybackCommand::payload() const noexcept
{
    return payload_;
}

std::optional<PlaybackCommandValidationError> validatePlaybackCommand(
    const PlaybackCommand& command) noexcept
{
    if (!command.requestId().isValid()) {
        return PlaybackCommandValidationError::InvalidRequestId;
    }

    return std::visit(
        [](const auto& payload) -> std::optional<PlaybackCommandValidationError> {
            using Command = std::decay_t<decltype(payload)>;

            if constexpr (std::is_same_v<Command, LoadMediaCommand>) {
                const QByteArray source = payload.source.toUtf8();
                if (source.isEmpty()) {
                    return PlaybackCommandValidationError::EmptyMediaSource;
                }
                if (source.contains('\0')) {
                    return PlaybackCommandValidationError::MediaSourceContainsNull;
                }
            } else if constexpr (std::is_same_v<Command, AddExternalSubtitleCommand>) {
                const QByteArray source = payload.source.toUtf8();
                if (source.isEmpty()) {
                    return PlaybackCommandValidationError::EmptyExternalSubtitleSource;
                }
                if (source.contains('\0')) {
                    return PlaybackCommandValidationError::ExternalSubtitleSourceContainsNull;
                }
            } else if constexpr (std::is_same_v<Command, SeekCommand>) {
                if (!std::isfinite(payload.seconds)) {
                    return PlaybackCommandValidationError::NonFiniteSeek;
                }
            } else if constexpr (std::is_same_v<Command, SetVolumeCommand>) {
                if (!std::isfinite(payload.percent) || payload.percent < 0.0) {
                    return PlaybackCommandValidationError::InvalidVolume;
                }
            } else if constexpr (std::is_same_v<Command, SetSpeedCommand>) {
                if (!std::isfinite(payload.rate) || payload.rate <= 0.0) {
                    return PlaybackCommandValidationError::InvalidSpeed;
                }
            } else if constexpr (std::is_same_v<Command, SetSubtitleDelayCommand>) {
                if (!std::isfinite(payload.seconds)
                    || payload.seconds < kSubtitleDelayMinimumSeconds
                    || payload.seconds > kSubtitleDelayMaximumSeconds) {
                    return PlaybackCommandValidationError::InvalidSubtitleDelay;
                }
            } else if constexpr (std::is_same_v<Command, SetAudioDelayCommand>) {
                if (!std::isfinite(payload.seconds)
                    || payload.seconds < kAudioDelayMinimumSeconds
                    || payload.seconds > kAudioDelayMaximumSeconds) {
                    return PlaybackCommandValidationError::InvalidAudioDelay;
                }
            } else if constexpr (std::is_same_v<Command, TrackSelectionCommand>) {
                if (payload.trackId.has_value() && *payload.trackId <= 0) {
                    return PlaybackCommandValidationError::InvalidTrackId;
                }
                if (payload.kind == TrackSelectionKind::Audio && !payload.trackId.has_value()) {
                    return PlaybackCommandValidationError::AudioTrackCannotBeDisabled;
                }
            }

            return std::nullopt;
        },
        command.payload());
}

} // namespace player::playback::domain
