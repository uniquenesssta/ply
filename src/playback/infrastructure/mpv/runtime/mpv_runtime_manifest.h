#pragma once

#include <QString>

namespace player::playback::mpv {

struct MpvRuntimeManifestInfo final
{
    QString mpvVersion;
    QString mpvTag;
    QString mpvCommit;
    QString ffmpegVersion;
};

class MpvRuntimeManifest final
{
public:
    [[nodiscard]] static bool load(
        const QString& manifestPath,
        MpvRuntimeManifestInfo& manifestInfo,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
