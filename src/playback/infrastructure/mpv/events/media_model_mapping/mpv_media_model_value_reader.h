#pragma once

#include "playback/infrastructure/mpv/properties/mpv_property_change.h"

#include <QString>
#include <QStringView>
#include <QVariantMap>

#include <optional>

class QVariant;

namespace player::playback::mpv {

class MpvMediaModelValueReader final
{
public:
    [[nodiscard]] static const QVariant* nodeValue(
        const MpvPropertyValue& value,
        bool* unavailable,
        QString* errorMessage);

    [[nodiscard]] static bool isInteger(const QVariant& value) noexcept;
    [[nodiscard]] static bool isNumeric(const QVariant& value) noexcept;

    [[nodiscard]] static bool optionalString(
        const QVariantMap& map,
        QStringView key,
        std::optional<QString>* output,
        QString* errorMessage);
    [[nodiscard]] static bool optionalBool(
        const QVariantMap& map,
        QStringView key,
        bool* output,
        QString* errorMessage);
    [[nodiscard]] static bool optionalInteger(
        const QVariantMap& map,
        QStringView key,
        std::optional<qint64>* output,
        QString* errorMessage);
    [[nodiscard]] static bool optionalDouble(
        const QVariantMap& map,
        QStringView key,
        std::optional<double>* output,
        QString* errorMessage);

    static void setError(QString* errorMessage, QString message);
};

} // namespace player::playback::mpv
