#pragma once

#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <QObject>
#include <QtGlobal>

#include <optional>
#include <variant>

class QString;

namespace player::playback::mpv {

class MpvHandle;

using MpvPropertyValue = std::variant<std::monostate, bool, double>;

struct MpvPropertyChange final
{
    MpvPropertyId id;
    MpvPropertyValue value;
};

class MpvPropertyObserver final : public QObject
{
    Q_OBJECT

public:
    explicit MpvPropertyObserver(MpvHandle& handle, QObject* parent = nullptr);
    ~MpvPropertyObserver() override;

    MpvPropertyObserver(const MpvPropertyObserver&) = delete;
    MpvPropertyObserver& operator=(const MpvPropertyObserver&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);
    [[nodiscard]] bool stop(QString* errorMessage = nullptr);
    [[nodiscard]] bool isObserving() const noexcept;

    [[nodiscard]] std::optional<MpvPropertyChange> decode(
        quint64 observationId,
        int rawFormat,
        const void* rawData,
        QString* errorMessage = nullptr) const;

private:
    [[nodiscard]] bool isOnOwningThread() const noexcept;
    [[nodiscard]] bool unobserveRegisteredProperties(QString* errorMessage);

    MpvHandle& handle_;
    bool observing_ = false;
};

} // namespace player::playback::mpv
