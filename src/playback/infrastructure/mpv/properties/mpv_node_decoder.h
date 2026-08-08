#pragma once

#include <QVariant>

#include <optional>

class QString;
struct mpv_node;

namespace player::playback::mpv {

class MpvNodeDecoder final
{
public:
    [[nodiscard]] static std::optional<QVariant> decode(
        const mpv_node& node,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
