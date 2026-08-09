#pragma once

#include <QString>

namespace player::playback::application::test_support {

[[nodiscard]] bool writeSilentPcmWav(
    const QString& path,
    int durationMilliseconds,
    QString* errorMessage = nullptr);

} // namespace player::playback::application::test_support
