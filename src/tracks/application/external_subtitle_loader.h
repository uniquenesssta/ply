#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QObject>
#include <QStringList>
#include <QtGlobal>

#include <optional>

namespace player::tracks::application {

// Loads external subtitle files (SRT/ASS/...) into the current media through
// the unified PlaybackCommand path. Idempotency is enforced here: repeated
// paths and paths already present in the authoritative track list are
// rejected, and the loaded track must surface through TrackListModel.
class ExternalSubtitleLoader final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canLoad READ canLoad NOTIFY stateChanged)

public:
    explicit ExternalSubtitleLoader(QObject* parent = nullptr);

    [[nodiscard]] bool canLoad() const noexcept;
    [[nodiscard]] QString lastRejectionReason() const;

    Q_INVOKABLE bool requestLoad(const QString& path);

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();
    void externalSubtitleLoadRequested(const QString& path);

private:
    [[nodiscard]] bool isDuplicate(const QString& canonicalPath) const;
    [[nodiscard]] static bool hasSupportedExtension(const QString& path);
    [[nodiscard]] static QString canonicalPathOf(const QString& path);

    bool canLoad_ = false;
    QString lastRejectionReason_;
    QStringList loadedExternalFilenames_;
};

} // namespace player::tracks::application
