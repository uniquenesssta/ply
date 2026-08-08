#include "playback/infrastructure/mpv/properties/mpv_property_observer.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/properties/mpv_node_decoder.h"

#include <mpv/client.h>

#include <QThread>
#include <QString>

namespace player::playback::mpv {
namespace {

mpv_format toMpvFormat(MpvPropertyFormat format) noexcept
{
    switch (format) {
    case MpvPropertyFormat::Flag:
        return MPV_FORMAT_FLAG;
    case MpvPropertyFormat::Double:
        return MPV_FORMAT_DOUBLE;
    case MpvPropertyFormat::Node:
        return MPV_FORMAT_NODE;
    }
    return MPV_FORMAT_NONE;
}

QString propertyLabel(const MpvPropertyDefinition& definition)
{
    return QString::fromUtf8(definition.name);
}

} // namespace

MpvPropertyObserver::MpvPropertyObserver(MpvHandle& handle, QObject* parent)
    : QObject(parent)
    , handle_(handle)
{
}

MpvPropertyObserver::~MpvPropertyObserver()
{
    if (observing_ && isOnOwningThread()) {
        (void)unobserveRegisteredProperties(nullptr);
    }
}

bool MpvPropertyObserver::start(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!isOnOwningThread()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("MpvPropertyObserver::start must run on the observer's owning Qt thread.");
        }
        return false;
    }

    if (observing_) {
        return true;
    }

    if (!handle_.isOpen() || !handle_.isInitialized() || handle_.nativeHandle() == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot observe mpv properties before the mpv handle is initialized.");
        }
        return false;
    }

    QList<quint64> registeredObservationIds;
    for (const MpvPropertyDefinition& definition : MpvPropertyRegistry::coreDefinitions()) {
        const int result = mpv_observe_property(
            handle_.nativeHandle(),
            definition.observationId,
            definition.name.constData(),
            toMpvFormat(definition.format));
        if (result >= 0) {
            registeredObservationIds.append(definition.observationId);
            continue;
        }

        for (quint64 observationId : registeredObservationIds) {
            (void)mpv_unobserve_property(handle_.nativeHandle(), observationId);
        }

        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to observe mpv property '%1': %2 (%3).")
                                .arg(
                                    propertyLabel(definition),
                                    QString::fromUtf8(mpv_error_string(result)))
                                .arg(result);
        }
        return false;
    }

    observing_ = true;
    return true;
}

bool MpvPropertyObserver::stop(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!observing_) {
        return true;
    }

    if (!isOnOwningThread()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("MpvPropertyObserver::stop must run on the observer's owning Qt thread.");
        }
        return false;
    }

    return unobserveRegisteredProperties(errorMessage);
}

bool MpvPropertyObserver::isObserving() const noexcept
{
    return observing_;
}

std::optional<MpvPropertyChange> MpvPropertyObserver::decode(
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

    const mpv_format expectedFormat = toMpvFormat(definition->format);
    if (rawFormat != expectedFormat) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unexpected mpv format for property '%1': expected %2, received %3.")
                                .arg(propertyLabel(*definition))
                                .arg(static_cast<int>(expectedFormat))
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

bool MpvPropertyObserver::isOnOwningThread() const noexcept
{
    return QThread::currentThread() == thread();
}

bool MpvPropertyObserver::unobserveRegisteredProperties(QString* errorMessage)
{
    if (!handle_.isOpen() || handle_.nativeHandle() == nullptr) {
        observing_ = false;
        return true;
    }

    int firstFailure = 0;
    QByteArray firstFailureName;
    for (const MpvPropertyDefinition& definition : MpvPropertyRegistry::coreDefinitions()) {
        const int result = mpv_unobserve_property(handle_.nativeHandle(), definition.observationId);
        if (result < 0 && firstFailure == 0) {
            firstFailure = result;
            firstFailureName = definition.name;
        }
    }

    observing_ = false;
    if (firstFailure >= 0) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Unable to stop observing mpv property '%1': %2 (%3).")
                            .arg(
                                QString::fromUtf8(firstFailureName),
                                QString::fromUtf8(mpv_error_string(firstFailure)))
                            .arg(firstFailure);
    }
    return false;
}

} // namespace player::playback::mpv
