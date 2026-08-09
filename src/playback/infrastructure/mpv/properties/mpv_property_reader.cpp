#include "mpv_property_reader.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/properties/mpv_property_decoder.h"

#include <mpv/client.h>

#include <QString>

#include <variant>

namespace player::playback::mpv {
namespace {

std::optional<MpvPropertyChange> unavailable(MpvPropertyId id)
{
    return MpvPropertyChange{id, std::monostate{}};
}

void setReadError(
    const MpvPropertyDefinition& definition,
    int result,
    QString* errorMessage)
{
    if (errorMessage == nullptr) {
        return;
    }

    *errorMessage = QStringLiteral("Unable to read mpv property '%1': %2 (%3).")
                        .arg(
                            QString::fromUtf8(definition.name),
                            QString::fromUtf8(mpv_error_string(result)))
                        .arg(result);
}

} // namespace

MpvPropertyReader::MpvPropertyReader(MpvHandle& handle) noexcept
    : handle_(handle)
{
}

std::optional<MpvPropertyChange> MpvPropertyReader::read(
    MpvPropertyId id,
    QString* errorMessage) const
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!handle_.isOpen() || !handle_.isInitialized() || handle_.nativeHandle() == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot read mpv properties before the mpv handle is initialized.");
        }
        return std::nullopt;
    }

    const MpvPropertyDefinition* definition = MpvPropertyRegistry::findById(id);
    if (definition == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unknown mpv property id: %1.").arg(static_cast<quint16>(id));
        }
        return std::nullopt;
    }

    const int rawFormat = MpvPropertyDecoder::rawFormat(definition->format);
    const auto decodeValue = [definition, rawFormat, errorMessage](const void* data) {
        return MpvPropertyDecoder::decode(
            definition->observationId,
            rawFormat,
            data,
            errorMessage);
    };

    int result = MPV_ERROR_GENERIC;
    switch (definition->format) {
    case MpvPropertyFormat::Flag: {
        int value = 0;
        result = mpv_get_property(
            handle_.nativeHandle(),
            definition->name.constData(),
            static_cast<mpv_format>(rawFormat),
            &value);
        if (result >= 0) {
            return decodeValue(&value);
        }
        break;
    }
    case MpvPropertyFormat::Double: {
        double value = 0.0;
        result = mpv_get_property(
            handle_.nativeHandle(),
            definition->name.constData(),
            static_cast<mpv_format>(rawFormat),
            &value);
        if (result >= 0) {
            return decodeValue(&value);
        }
        break;
    }
    case MpvPropertyFormat::String: {
        char* value = nullptr;
        result = mpv_get_property(
            handle_.nativeHandle(),
            definition->name.constData(),
            static_cast<mpv_format>(rawFormat),
            &value);
        if (result >= 0) {
            auto decoded = decodeValue(&value);
            mpv_free(value);
            return decoded;
        }
        break;
    }
    case MpvPropertyFormat::Node: {
        mpv_node value{};
        result = mpv_get_property(
            handle_.nativeHandle(),
            definition->name.constData(),
            static_cast<mpv_format>(rawFormat),
            &value);
        if (result >= 0) {
            auto decoded = decodeValue(&value);
            mpv_free_node_contents(&value);
            return decoded;
        }
        break;
    }
    }

    if (result == MPV_ERROR_PROPERTY_UNAVAILABLE) {
        return unavailable(id);
    }

    setReadError(*definition, result, errorMessage);
    return std::nullopt;
}

} // namespace player::playback::mpv
