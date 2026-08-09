#pragma once

#include "playback/infrastructure/mpv/properties/mpv_property_change.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <QObject>

#include <optional>

class QString;

namespace player::playback::mpv {

class MpvHandle;

class MpvPropertyReader final : public QObject
{
public:
    explicit MpvPropertyReader(MpvHandle& handle, QObject* parent = nullptr) noexcept;

    MpvPropertyReader(const MpvPropertyReader&) = delete;
    MpvPropertyReader& operator=(const MpvPropertyReader&) = delete;

    [[nodiscard]] std::optional<MpvPropertyChange> read(
        MpvPropertyId id,
        QString* errorMessage = nullptr) const;

private:
    [[nodiscard]] bool isOnOwningThread() const noexcept;

    MpvHandle& handle_;
};

} // namespace player::playback::mpv
