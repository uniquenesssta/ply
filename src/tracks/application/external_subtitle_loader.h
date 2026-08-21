#pragma once

#include "foundation/ids/request_id.h"
#include "playback/domain/commands/external_subtitle_command.h"
#include "playback/domain/state/media_generation.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUrl>

#include <functional>
#include <optional>

namespace player::tracks::application {

class ExternalSubtitleLoader final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastErrorKey READ lastErrorKey NOTIFY lastErrorKeyChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    using SubmitExternalSubtitle = std::function<std::optional<player::ids::RequestId>(
        const player::playback::domain::AddExternalSubtitleCommand&)>;

    explicit ExternalSubtitleLoader(
        SubmitExternalSubtitle submitExternalSubtitle,
        QObject* parent = nullptr);

    [[nodiscard]] QString lastErrorKey() const;
    [[nodiscard]] bool loading() const noexcept;

    Q_INVOKABLE bool loadLocalSubtitle(const QUrl& sourceUrl);

    void acceptRequestResult(
        player::ids::RequestId requestId,
        player::playback::domain::MediaGeneration generation,
        bool succeeded,
        const QString& diagnostic);
    void beginShutdown() noexcept;

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void lastErrorKeyChanged();
    void loadingChanged();

private:
    struct PendingLoad final
    {
        QString sourceKey;
        player::playback::domain::MediaGeneration generation;
    };

    [[nodiscard]] static QString localPathKey(const QString& path);
    [[nodiscard]] bool containsSourceKey(const QString& sourceKey) const;
    [[nodiscard]] bool reject(QString errorKey);
    void clearPendingLoads() noexcept;
    void setLastErrorKey(QString errorKey);

    SubmitExternalSubtitle submitExternalSubtitle_;
    QHash<quint64, PendingLoad> pendingLoads_;
    QSet<QString> acceptedSourceKeys_;
    QSet<QString> snapshotSourceKeys_;
    player::playback::domain::MediaGeneration generation_;
    QString lastErrorKey_;
    bool shuttingDown_ = false;
};

} // namespace player::tracks::application
