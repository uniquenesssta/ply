#include "playback/infrastructure/mpv/properties/mpv_property_observer.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/properties/mpv_property_decoder.h"

#include <mpv/client.h>

#include <QThread>
#include <QString>

namespace player::playback::mpv {
namespace {

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
            static_cast<mpv_format>(MpvPropertyDecoder::rawFormat(definition.format)));
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
    return MpvPropertyDecoder::decode(
        observationId,
        rawFormat,
        rawData,
        errorMessage);
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
