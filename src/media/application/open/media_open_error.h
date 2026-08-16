#pragma once

#include <QtGlobal>

class QString;

namespace player::media::application {

enum class MediaOpenError : quint8
{
    None = 0,
    NotLocalFile,
    NotFound,
    NotRegularFile,
    NotReadable,
    CanonicalizationFailed,
    EmptyUrl,
    InvalidUrl,
    UnsupportedUrlScheme,
    SubmissionRejected,
};

[[nodiscard]] QString mediaOpenErrorKey(MediaOpenError error);

} // namespace player::media::application
