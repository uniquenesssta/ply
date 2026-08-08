#pragma once

#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <QVariant>

#include <variant>

namespace player::playback::mpv {

using MpvPropertyValue = std::variant<std::monostate, bool, double, QVariant>;

struct MpvPropertyChange final
{
    MpvPropertyId id;
    MpvPropertyValue value;
};

} // namespace player::playback::mpv
