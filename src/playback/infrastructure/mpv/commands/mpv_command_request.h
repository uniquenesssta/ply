#pragma once

#include <QString>
#include <QtGlobal>

#include <optional>
#include <variant>

namespace player::playback::mpv {

enum class MpvSeekMode
{
    Absolute,
    Relative,
};

enum class MpvTrackSelectionKind
{
    Audio,
    Subtitle,
};

struct MpvLoadRequest final
{
    QString source;
};

struct MpvPlayRequest final
{
};

struct MpvPauseRequest final
{
};

struct MpvStopRequest final
{
};

struct MpvSeekRequest final
{
    double seconds = 0.0;
    MpvSeekMode mode = MpvSeekMode::Absolute;
};

struct MpvVolumeRequest final
{
    double percent = 100.0;
};

struct MpvMuteRequest final
{
    bool muted = false;
};

struct MpvSpeedRequest final
{
    double rate = 1.0;
};

struct MpvTrackSelectionRequest final
{
    MpvTrackSelectionKind kind = MpvTrackSelectionKind::Audio;
    std::optional<qint64> trackId;
};

struct MpvExternalSubtitleRequest final
{
    QString source;
};

struct MpvSubtitleDelayRequest final
{
    double seconds = 0.0;
};

struct MpvAudioDelayRequest final
{
    double seconds = 0.0;
};

using MpvCommandRequest = std::variant<
    MpvLoadRequest,
    MpvPlayRequest,
    MpvPauseRequest,
    MpvStopRequest,
    MpvSeekRequest,
    MpvVolumeRequest,
    MpvMuteRequest,
    MpvSpeedRequest,
    MpvTrackSelectionRequest,
    MpvExternalSubtitleRequest,
    MpvSubtitleDelayRequest,
    MpvAudioDelayRequest>;

} // namespace player::playback::mpv
