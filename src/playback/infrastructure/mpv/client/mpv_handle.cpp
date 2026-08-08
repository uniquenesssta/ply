#include "playback/infrastructure/mpv/client/mpv_handle.h"

#include <mpv/client.h>

#include <QString>

#include <utility>

namespace player::playback::mpv {

std::unique_ptr<MpvHandle> MpvHandle::create(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    mpv_handle* handle = mpv_create();
    if (handle == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("mpv_create returned null.");
        }
        return {};
    }

    return std::unique_ptr<MpvHandle>(new MpvHandle(handle));
}

MpvHandle::MpvHandle(mpv_handle* handle) noexcept
    : handle_(handle)
{
}

MpvHandle::~MpvHandle()
{
    close();
}

bool MpvHandle::initialize(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (handle_ == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot initialize a closed mpv handle.");
        }
        return false;
    }

    if (initialized_) {
        return true;
    }

    const int result = mpv_initialize(handle_);
    if (result < 0) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("mpv_initialize failed: %1 (%2).")
                                .arg(QString::fromUtf8(mpv_error_string(result)))
                                .arg(result);
        }
        close();
        return false;
    }

    initialized_ = true;
    return true;
}

void MpvHandle::close() noexcept
{
    mpv_handle* handle = std::exchange(handle_, nullptr);
    const bool wasInitialized = std::exchange(initialized_, false);

    if (handle == nullptr) {
        return;
    }

    if (wasInitialized) {
        mpv_terminate_destroy(handle);
        return;
    }

    mpv_destroy(handle);
}

bool MpvHandle::isOpen() const noexcept
{
    return handle_ != nullptr;
}

bool MpvHandle::isInitialized() const noexcept
{
    return initialized_;
}

mpv_handle* MpvHandle::nativeHandle() noexcept
{
    return handle_;
}

const mpv_handle* MpvHandle::nativeHandle() const noexcept
{
    return handle_;
}

} // namespace player::playback::mpv
