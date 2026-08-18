#pragma once

#include "playlist/domain/playlist_entry_id.h"

#include <QObject>
#include <QSet>
#include <QtGlobal>

#include <optional>

namespace player::playback::domain {
class PlaybackSnapshot;
}

namespace player::playlist::application {
class PlaylistController;
}

namespace player::playlist::presentation {

class PlaylistEntryPlaybackState final : public QObject
{
    Q_OBJECT

public:
    explicit PlaylistEntryPlaybackState(
        application::PlaylistController& controller,
        QObject* parent = nullptr);

    [[nodiscard]] bool isPendingLoading(domain::PlaylistEntryId entryId) const noexcept;
    [[nodiscard]] bool isUnavailable(domain::PlaylistEntryId entryId) const noexcept;

signals:
    void entryStatesChanged();

public slots:
    void acceptPlaybackSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

private slots:
    void synchronizeWithPlaylist();

private:
    [[nodiscard]] bool setPendingLoading(
        std::optional<domain::PlaylistEntryId> entryId) noexcept;
    [[nodiscard]] bool clearUnavailable(domain::PlaylistEntryId entryId) noexcept;
    [[nodiscard]] bool markUnavailable(domain::PlaylistEntryId entryId) noexcept;

    application::PlaylistController& controller_;
    std::optional<domain::PlaylistEntryId> pendingLoadingId_;
    QSet<quint64> unavailableEntryIds_;
};

} // namespace player::playlist::presentation
