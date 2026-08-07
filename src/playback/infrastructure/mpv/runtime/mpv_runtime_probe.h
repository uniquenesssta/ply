#pragma once

#include "playback/infrastructure/mpv/runtime/mpv_runtime_manifest.h"

#include <QString>

namespace player::playback::mpv {

struct MpvRuntimeInfo final
{
    QString runtimeLibraryPath;
    QString manifestPath;
    MpvRuntimeManifestInfo manifest;
    int clientApiMajor = 0;
    int clientApiMinor = 0;
};

class MpvRuntimeProbe final
{
public:
    [[nodiscard]] static bool probe(
        MpvRuntimeInfo& runtimeInfo,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
