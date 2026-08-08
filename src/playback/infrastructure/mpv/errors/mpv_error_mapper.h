#pragma once

#include <QString>

namespace player::playback::mpv {

enum class MpvErrorCode
{
    Success,
    EventQueueFull,
    NoMemory,
    Uninitialized,
    InvalidParameter,
    OptionNotFound,
    OptionFormat,
    OptionError,
    PropertyNotFound,
    PropertyFormat,
    PropertyUnavailable,
    PropertyError,
    Command,
    LoadingFailed,
    AudioOutputInitFailed,
    VideoOutputInitFailed,
    NothingToPlay,
    UnknownFormat,
    Unsupported,
    NotImplemented,
    Generic,
    Unknown,
};

struct MpvError final
{
    MpvErrorCode code = MpvErrorCode::Success;
    int rawCode = 0;
    QString message;

    [[nodiscard]] bool isSuccess() const noexcept
    {
        return rawCode >= 0;
    }
};

class MpvErrorMapper final
{
public:
    [[nodiscard]] static MpvError map(int rawCode);
};

} // namespace player::playback::mpv
