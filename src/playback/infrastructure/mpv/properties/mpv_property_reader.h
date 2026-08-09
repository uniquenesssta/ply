#pragma once

#include "playback/infrastructure/mpv/properties/mpv_property_change.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <optional>

class QString;

namespace player::playback::mpv {

class MpvHandle;

class MpvPropertyReader final
{
public:
    explicit MpvPropertyReader(MpvHandle& handle) noexcept;

    MpvPropertyReader(const MpvPropertyReader&) = delete;
    MpvPropertyReader& operator=(const MpvPropertyReader&) = delete;

    [[nodiscard]] std::optional<MpvPropertyChange> read(
        MpvPropertyId id,
        QString* errorMessage = nullptr) const;

private:
    MpvHandle& handle_;
};

} // namespace player::playback::mpv
