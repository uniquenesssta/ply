#pragma once

#include <QString>

namespace player::playback::infrastructure::mpv::render {

class OpenGlProcResolver final {
public:
    [[nodiscard]] static bool validateCurrentContext(QString* errorMessage = nullptr);

    [[nodiscard]] static void* resolveCurrent(
        const char* procedureName,
        QString* errorMessage = nullptr);

    [[nodiscard]] static void* resolveForMpv(
        void* expectedContext,
        const char* procedureName) noexcept;
};

} // namespace player::playback::infrastructure::mpv::render
