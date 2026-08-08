#pragma once

#include "playback/infrastructure/mpv/commands/mpv_command_request.h"

#include <QObject>
#include <QtGlobal>

class QString;

namespace player::playback::mpv {

class MpvHandle;

class MpvCommandExecutor final : public QObject
{
public:
    explicit MpvCommandExecutor(MpvHandle& handle, QObject* parent = nullptr);

    MpvCommandExecutor(const MpvCommandExecutor&) = delete;
    MpvCommandExecutor& operator=(const MpvCommandExecutor&) = delete;

    [[nodiscard]] bool submit(
        quint64 requestId,
        const MpvCommandRequest& request,
        QString* errorMessage = nullptr);

private:
    [[nodiscard]] bool isOnOwningThread() const noexcept;

    MpvHandle& handle_;
};

} // namespace player::playback::mpv
