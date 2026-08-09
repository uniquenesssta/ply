#include "mpv_media_model_value_reader.h"

#include <QMetaType>
#include <QString>
#include <QVariant>

#include <cmath>
#include <utility>
#include <variant>

namespace player::playback::mpv {

void MpvMediaModelValueReader::setError(QString* errorMessage, QString message)
{
    if (errorMessage != nullptr) {
        *errorMessage = std::move(message);
    }
}

const QVariant* MpvMediaModelValueReader::nodeValue(
    const MpvPropertyValue& value,
    bool* unavailable,
    QString* errorMessage)
{
    *unavailable = false;
    if (std::holds_alternative<std::monostate>(value)) {
        *unavailable = true;
        return nullptr;
    }

    const QVariant* node = std::get_if<QVariant>(&value);
    if (node == nullptr) {
        setError(errorMessage, QStringLiteral("Complex playback property is not backed by an mpv node value."));
        return nullptr;
    }
    if (!node->isValid()) {
        *unavailable = true;
        return nullptr;
    }
    return node;
}

bool MpvMediaModelValueReader::isInteger(const QVariant& value) noexcept
{
    switch (value.metaType().id()) {
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        return true;
    default:
        return false;
    }
}

bool MpvMediaModelValueReader::isNumeric(const QVariant& value) noexcept
{
    return isInteger(value) || value.metaType().id() == QMetaType::Double;
}

bool MpvMediaModelValueReader::optionalString(
    const QVariantMap& map,
    QStringView key,
    std::optional<QString>* output,
    QString* errorMessage)
{
    const auto it = map.constFind(key.toString());
    if (it == map.cend() || !it->isValid()) {
        output->reset();
        return true;
    }
    if (it->metaType().id() != QMetaType::QString) {
        setError(errorMessage, QStringLiteral("Field '%1' must be a string when present.").arg(key));
        return false;
    }
    *output = it->toString();
    return true;
}

bool MpvMediaModelValueReader::optionalBool(
    const QVariantMap& map,
    QStringView key,
    bool* output,
    QString* errorMessage)
{
    const auto it = map.constFind(key.toString());
    if (it == map.cend() || !it->isValid()) {
        *output = false;
        return true;
    }
    if (it->metaType().id() != QMetaType::Bool) {
        setError(errorMessage, QStringLiteral("Field '%1' must be a boolean when present.").arg(key));
        return false;
    }
    *output = it->toBool();
    return true;
}

bool MpvMediaModelValueReader::optionalInteger(
    const QVariantMap& map,
    QStringView key,
    std::optional<qint64>* output,
    QString* errorMessage)
{
    const auto it = map.constFind(key.toString());
    if (it == map.cend() || !it->isValid()) {
        output->reset();
        return true;
    }
    if (!isInteger(*it)) {
        setError(errorMessage, QStringLiteral("Field '%1' must be an integer when present.").arg(key));
        return false;
    }
    bool converted = false;
    const qint64 value = it->toLongLong(&converted);
    if (!converted) {
        setError(errorMessage, QStringLiteral("Field '%1' is outside the supported integer range.").arg(key));
        return false;
    }
    *output = value;
    return true;
}

bool MpvMediaModelValueReader::optionalDouble(
    const QVariantMap& map,
    QStringView key,
    std::optional<double>* output,
    QString* errorMessage)
{
    const auto it = map.constFind(key.toString());
    if (it == map.cend() || !it->isValid()) {
        output->reset();
        return true;
    }
    if (!isNumeric(*it)) {
        setError(errorMessage, QStringLiteral("Field '%1' must be numeric when present.").arg(key));
        return false;
    }
    bool converted = false;
    const double value = it->toDouble(&converted);
    if (!converted || !std::isfinite(value)) {
        setError(errorMessage, QStringLiteral("Field '%1' contains a non-finite numeric value.").arg(key));
        return false;
    }
    *output = value;
    return true;
}

} // namespace player::playback::mpv
