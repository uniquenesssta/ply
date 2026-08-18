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
            [](const SelectTrackCommand& selection) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvSelectTrackRequest{
                    selection.kind,
                    selection.trackId}};
            },
            [](const SetSubtitleDelayCommand& delay) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvSubtitleDelayRequest{delay.seconds}};
            },
            [](const SetAudioDelayCommand& delay) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvAudioDelayRequest{delay.seconds}};
            },
            [](const LoadExternalSubtitleCommand& subtitle) -> std::optional<MpvCommandRequest> {
                return MpvCommandRequest{MpvExternalSubtitleRequest{subtitle.path}};
            },
            [](const LifecycleCommand&) -> std::optional<MpvCommandRequest> {
                return std::nullopt;
            }},
        payload);
}

} // namespace player::playback::mpv
