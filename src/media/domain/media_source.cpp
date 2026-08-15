#include "media/media_source.h"

#include <utility>

namespace player::media::domain {

MediaSource MediaSource::localFile(QString canonicalPath)
{
    return MediaSource{MediaSourceKind::LocalFile, std::move(canonicalPath)};
}

MediaSource::MediaSource(MediaSourceKind kind, QString location)
    : kind_(kind)
    , location_(std::move(location))
{
}

MediaSourceKind MediaSource::kind() const noexcept
{
    return kind_;
}

const QString& MediaSource::location() const noexcept
{
    return location_;
}

bool MediaSource::isValid() const noexcept
{
    return !location_.isEmpty();
}

} // namespace player::media::domain
