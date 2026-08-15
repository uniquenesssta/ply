#pragma once

#include "media/application/open/media_open_error.h"
#include "media/domain/media_source.h"

#include <QUrl>

#include <variant>

namespace player::media::application {

using LocalMediaValidationResult =
    std::variant<player::media::domain::MediaSource, MediaOpenError>;

class LocalMediaValidator final
{
public:
    [[nodiscard]] static LocalMediaValidationResult validate(const QUrl& sourceUrl);
};

} // namespace player::media::application
