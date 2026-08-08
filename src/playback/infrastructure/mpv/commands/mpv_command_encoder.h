#pragma once

#include "playback/infrastructure/mpv/commands/mpv_command_request.h"

#include <QByteArray>
#include <QList>

#include <optional>

class QString;

namespace player::playback::mpv {

class MpvCommandEncoder final
{
public:
    [[nodiscard]] static std::optional<QList<QByteArray>> encode(
        const MpvCommandRequest& request,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
