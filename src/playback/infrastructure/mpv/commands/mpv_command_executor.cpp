#include "playback/infrastructure/mpv/commands/mpv_command_executor.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/commands/mpv_command_encoder.h"

#include <mpv/client.h>

#include <QThread>
#include <QString>

#include <vector>

namespace player::playback::mpv {

MpvCommandExecutor::MpvCommandExecutor(MpvHandle& handle, QObject* parent)
    : QObject(parent)
    , handle_(handle)
{
}

bool MpvCommandExecutor::submit(
    quint64 requestId,
    const MpvCommandRequest& request,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!isOnOwningThread()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("MpvCommandExecutor::submit must run on the executor's owning Qt thread.");
        }
        return false;
    }

    if (!handle_.isOpen() || !handle_.isInitialized() || handle_.nativeHandle() == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot submit an mpv command before the mpv handle is initialized.");
        }
        return false;
    }

    const auto encoded = MpvCommandEncoder::encode(request, errorMessage);
    if (!encoded.has_value()) {
        return false;
    }

    std::vector<const char*> arguments;
    arguments.reserve(static_cast<std::size_t>(encoded->size()) + 1U);
    for (const QByteArray& argument : *encoded) {
        arguments.push_back(argument.constData());
    }
    arguments.push_back(nullptr);

    const int result = mpv_command_async(
        handle_.nativeHandle(),
        static_cast<uint64_t>(requestId),
        arguments.data());
    if (result >= 0) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("mpv_command_async submission failed: %1 (%2).")
                            .arg(QString::fromUtf8(mpv_error_string(result)))
                            .arg(result);
    }
    return false;
}

bool MpvCommandExecutor::isOnOwningThread() const noexcept
{
    return QThread::currentThread() == thread();
}

} // namespace player::playback::mpv
