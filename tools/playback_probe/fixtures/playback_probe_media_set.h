#pragma once

#include <QString>
#include <QTemporaryDir>

namespace player::tools::playback_probe {

class PlaybackProbeMediaSet final
{
public:
    PlaybackProbeMediaSet() = default;

    PlaybackProbeMediaSet(const PlaybackProbeMediaSet&) = delete;
    PlaybackProbeMediaSet& operator=(const PlaybackProbeMediaSet&) = delete;

    [[nodiscard]] bool create(QString* errorMessage = nullptr);

    [[nodiscard]] const QString& mediaA() const noexcept;
    [[nodiscard]] const QString& mediaB() const noexcept;
    [[nodiscard]] const QString& eofMedia() const noexcept;
    [[nodiscard]] const QString& missingMedia() const noexcept;

private:
    QTemporaryDir directory_;
    QString mediaA_;
    QString mediaB_;
    QString eofMedia_;
    QString missingMedia_;
};

} // namespace player::tools::playback_probe
