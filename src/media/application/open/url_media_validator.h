#pragma once

#include "media/application/open/media_open_error.h"
#include "media/domain/media_source.h"

#include <QString>

#include <variant>

namespace player::media::application {

using UrlMediaValidationResult =
    std::variant<player::media::domain::MediaSource, MediaOpenError>;

class UrlMediaValidator final
{
public:
    [[nodiscard]] static UrlMediaValidationResult validate(const QString& sourceText);
};

} // namespace player::media::application
