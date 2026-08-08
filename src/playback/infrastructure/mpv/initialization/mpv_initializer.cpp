#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/initialization/mpv_option_profile.h"

#include <mpv/client.h>

#include <QString>

namespace player::playback::mpv {
namespace {

bool validateOption(const MpvOption& option, QString* errorMessage)
{
    if (!option.name.isEmpty() && !option.name.contains('\0') && !option.value.contains('\0')) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Invalid mpv option profile entry: option names must be non-empty and option strings cannot contain embedded null bytes.");
    }
    return false;
}

bool applyOption(
    MpvHandle& handle,
    const MpvOption& option,
    QString* errorMessage)
{
    if (!validateOption(option, errorMessage)) {
        return false;
    }

    const int result = mpv_set_option_string(
        handle.nativeHandle(),
        option.name.constData(),
        option.value.constData());
    if (result >= 0) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Unable to apply mpv option '%1=%2': %3 (%4).")
                            .arg(
                                QString::fromUtf8(option.name),
                                QString::fromUtf8(option.value),
                                QString::fromUtf8(mpv_error_string(result)))
                            .arg(result);
    }
    return false;
}

} // namespace

bool MpvInitializer::initializeProduct(MpvHandle& handle, QString* errorMessage)
{
    return initialize(handle, MpvOptionProfile::productDefaults(), errorMessage);
}

bool MpvInitializer::initialize(
    MpvHandle& handle,
    const MpvOptionProfile& profile,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!handle.isOpen()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot initialize a closed mpv handle.");
        }
        return false;
    }

    if (handle.isInitialized()) {
        return true;
    }

    for (const MpvOption& option : profile.options()) {
        if (!applyOption(handle, option, errorMessage)) {
            handle.close();
            return false;
        }
    }

    return handle.initialize(errorMessage);
}

} // namespace player::playback::mpv
