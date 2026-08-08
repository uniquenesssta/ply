#include "playback/infrastructure/mpv/errors/mpv_error_mapper.h"

#include <mpv/client.h>

namespace player::playback::mpv {

MpvError MpvErrorMapper::map(int rawCode)
{
    MpvError result;
    result.rawCode = rawCode;
    result.message = QString::fromUtf8(mpv_error_string(rawCode));

    if (rawCode >= 0) {
        result.code = MpvErrorCode::Success;
        return result;
    }

    switch (rawCode) {
    case MPV_ERROR_EVENT_QUEUE_FULL:
        result.code = MpvErrorCode::EventQueueFull;
        break;
    case MPV_ERROR_NOMEM:
        result.code = MpvErrorCode::NoMemory;
        break;
    case MPV_ERROR_UNINITIALIZED:
        result.code = MpvErrorCode::Uninitialized;
        break;
    case MPV_ERROR_INVALID_PARAMETER:
        result.code = MpvErrorCode::InvalidParameter;
        break;
    case MPV_ERROR_OPTION_NOT_FOUND:
        result.code = MpvErrorCode::OptionNotFound;
        break;
    case MPV_ERROR_OPTION_FORMAT:
        result.code = MpvErrorCode::OptionFormat;
        break;
    case MPV_ERROR_OPTION_ERROR:
        result.code = MpvErrorCode::OptionError;
        break;
    case MPV_ERROR_PROPERTY_NOT_FOUND:
        result.code = MpvErrorCode::PropertyNotFound;
        break;
    case MPV_ERROR_PROPERTY_FORMAT:
        result.code = MpvErrorCode::PropertyFormat;
        break;
    case MPV_ERROR_PROPERTY_UNAVAILABLE:
        result.code = MpvErrorCode::PropertyUnavailable;
        break;
    case MPV_ERROR_PROPERTY_ERROR:
        result.code = MpvErrorCode::PropertyError;
        break;
    case MPV_ERROR_COMMAND:
        result.code = MpvErrorCode::Command;
        break;
    case MPV_ERROR_LOADING_FAILED:
        result.code = MpvErrorCode::LoadingFailed;
        break;
    case MPV_ERROR_AO_INIT_FAILED:
        result.code = MpvErrorCode::AudioOutputInitFailed;
        break;
    case MPV_ERROR_VO_INIT_FAILED:
        result.code = MpvErrorCode::VideoOutputInitFailed;
        break;
    case MPV_ERROR_NOTHING_TO_PLAY:
        result.code = MpvErrorCode::NothingToPlay;
        break;
    case MPV_ERROR_UNKNOWN_FORMAT:
        result.code = MpvErrorCode::UnknownFormat;
        break;
    case MPV_ERROR_UNSUPPORTED:
        result.code = MpvErrorCode::Unsupported;
        break;
    case MPV_ERROR_NOT_IMPLEMENTED:
        result.code = MpvErrorCode::NotImplemented;
        break;
    case MPV_ERROR_GENERIC:
        result.code = MpvErrorCode::Generic;
        break;
    default:
        result.code = MpvErrorCode::Unknown;
        break;
    }

    return result;
}

} // namespace player::playback::mpv
