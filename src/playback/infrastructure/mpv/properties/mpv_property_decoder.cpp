#include "mpv_property_decoder.h"

#include "playback/infrastructure/mpv/properties/mpv_node_decoder.h"

#include <mpv/client.h>

#include <QString>

#include <variant>

namespace player::playback::mpv {
namespace {

QString propertyLabel(const MpvPropertyDefinition& definition)
{
    return QString::fromUtf8(definition.name);
}

} // namespace

int MpvPropertyDecoder::rawFormat(MpvPropertyFormat format) noexcept
{
    switch (format) {
    case MpvPropertyFormat::Flag:
        return MPV_FORMAT_FLAG;
    case MpvPropertyFormat::Double:
        return MPV_FORMAT_DOUBLE;
    case MpvPropertyFormat::String:
        return MPV_FORMAT_STRING;
    case MpvPropertyFormat::Node:
        return MPV_FORMAT_NODE;
    }
    return MPV_FORMAT_NONE;
}

std::optional<MpvPropertyChange> MpvPropertyDecoder::decode(
    quint64 observationId,
    int rawFormat,
    const void* rawData,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    const MpvPropertyDefinition* definition = MpvPropertyRegistry::findByObservationId(observationId);
    if (definition == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unknown mpv property observation id: %1.").arg(observationId);
        }
        return std::nullopt;
    }

    if (rawFormat == MPV_FORMAT_NONE || rawData == nullptr) {
        return MpvPropertyChange{definition->id, std::monostate{}};
    }

    const int expectedFormat = MpvPropertyDecoder::rawFormat(definition->format);
    if (rawFormat != expectedFormat) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unexpected mpv format for property '%1': expected %2, received %3.")
                                .arg(propertyLabel(*definition))
                                .arg(expectedFormat)
                                .arg(rawFormat);
        }
        return std::nullopt;
    }

    switch (definition->format) {
    case MpvPropertyFormat::Flag:
        return MpvPropertyChange{
            definition->id,
            (*static_cast<const int*>(rawData) != 0)};
    case MpvPropertyFormat::Double:
        return MpvPropertyChange{
            definition->id,
            *static_cast<const double*>(rawData)};
    case MpvPropertyFormat::String: {
        const auto* value = static_cast<char* const*>(rawData);
        if (value == nullptr || *value == nullptr) {
            return MpvPropertyChange{definition->id, std::monostate{}};
        }
        return MpvPropertyChange{
            definition->id,
            QString::fromUtf8(*value)};
    }
    case MpvPropertyFormat::Node: {
        const auto* node = static_cast<const mpv_node*>(rawData);
        auto decoded = MpvNodeDecoder::decode(*node, errorMessage);
        if (!decoded.has_value()) {
            return std::nullopt;
        }
        return MpvPropertyChange{definition->id, *decoded};
    }
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Unsupported mpv property format.");
    }
    return std::nullopt;
}

} // namespace player::playback::mpv
