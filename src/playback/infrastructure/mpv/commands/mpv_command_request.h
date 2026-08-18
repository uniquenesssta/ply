#pragma once

#include "playback/domain/models/track_descriptor.h"

#include <QString>

#include <optional>
#include <variant>

namespace player::playback::mpv {

enum class MpvSeekMode
{
    Absolute,
    Relative,
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

// Selects a backend track by stable id, or disables the family when trackId is
// empty (encoded as 'no').
struct MpvSelectTrackRequest final
{
    player::playback::domain::TrackKind kind =
        player::playback::domain::TrackKind::Video;
    std::optional<qint64> trackId;
};

struct MpvSubtitleDelayRequest final
{
    double seconds = 0.0;
};

struct MpvAudioDelayRequest final
{
    double seconds = 0.0;
};

struct MpvExternalSubtitleRequest final
{
    QString path;
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
    MpvSelectTrackRequest,
    MpvSubtitleDelayRequest,
    MpvAudioDelayRequest,
    MpvExternalSubtitleRequest>;

} // namespace player::playback::mpv
