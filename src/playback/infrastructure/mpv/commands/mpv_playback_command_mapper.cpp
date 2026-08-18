#include "mpv_playback_command_mapper.h"

#include <variant>

namespace player::playback::mpv {
namespace {

using namespace player::playback::domain;

template <typename... T>
struct Overloaded : T...
{
    using T::operator()...;
};

template <typename... T>
Overloaded(T...) -> Overloaded<T...>;

} // namespace

std::optional<MpvCommandRequest> MpvPlaybackCommandMapper::map(
    const PlaybackCommandPayload& payload)
{
    return std::visit(
        Overloaded{
            [](const LoadMediaCommand& load) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvLoadRequest{load.source}};
            },
            [](const TransportCommand& transport) -> std::optional<MpvCommandRequest> {
                switch (transport.action) {
                case TransportAction::Play:
                    return MpvCommandRequest{MpvPlayRequest{}};
                case TransportAction::Pause:
                    return MpvCommandRequest{MpvPauseRequest{}};
                case TransportAction::Stop:
                    return MpvCommandRequest{MpvStopRequest{}};
                }
                return std::nullopt;
            },
            [](const SeekCommand& seek) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvSeekRequest{
                    seek.seconds,
                    seek.mode == SeekMode::Absolute ? MpvSeekMode::Absolute : MpvSeekMode::Relative}};
            },
            [](const SetVolumeCommand& volume) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvVolumeRequest{volume.percent}};
            },
            [](const SetMutedCommand& mute) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvMuteRequest{mute.muted}};
            },
            [](const SetSpeedCommand& speed) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvSpeedRequest{speed.rate}};
            },
            [](const TrackSelectionCommand& selection) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvTrackSelectionRequest{
                    selection.kind == TrackSelectionKind::Audio
                        ? MpvTrackSelectionKind::Audio
                        : MpvTrackSelectionKind::Subtitle,
                    selection.trackId}};
            },
            [](const LifecycleCommand&) -> std::optional<MpvCommandRequest> {
                return std::nullopt;
            }},
        payload);
}

} // namespace player::playback::mpv
