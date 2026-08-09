#pragma once

#include "playback/infrastructure/mpv/properties/mpv_property_change.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <QtGlobal>

#include <optional>

class QString;

namespace player::playback::mpv {

class MpvPropertyDecoder final
{
public:
    [[nodiscard]] static int rawFormat(MpvPropertyFormat format) noexcept;

    [[nodiscard]] static std::optional<MpvPropertyChange> decode(
        quint64 observationId,
        int rawFormat,
        const void* rawData,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
