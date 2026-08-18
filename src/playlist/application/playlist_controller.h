#pragma once

#include "media/domain/media_source.h"
#include "playlist/domain/playlist.h"

#include <QList>
#include <QObject>
#include <QtGlobal>

#include <functional>
#include <optional>

namespace player::playlist::application {

class PlaylistAdvanceArbiter;
class PlaylistMutation;

class PlaylistController final : public QObject
{
    Q_OBJECT

public:
    using SubmitMediaLoad = std::function<bool(const media::domain::MediaSource&)>;
    using SubmitMediaStop = std::function<bool()>;

    PlaylistController(
        domain::Playlist& playlist,
        PlaylistMutation& mutation,
        SubmitMediaLoad submitMediaLoad,
        QObject* parent = nullptr);
    PlaylistController(
        domain::Playlist& playlist,
        PlaylistMutation& mutation,
        SubmitMediaLoad submitMediaLoad,
        SubmitMediaStop submitMediaStop,
        QObject* parent,
        PlaylistAdvanceArbiter* advanceArbiter = nullptr);

    [[nodiscard]] const domain::Playlist& playlist() const noexcept;
    [[nodiscard]] domain::PlaylistSnapshot snapshot() const;

    [[nodiscard]] bool openSource(const media::domain::MediaSource& source);
    [[nodiscard]] bool openSources(const QList<media::domain::MediaSource>& sources);
    [[nodiscard]] bool appendSources(const QList<media::domain::MediaSource>& sources);
    [[nodiscard]] bool insertSource(
        const media::domain::MediaSource& source,
        int targetIndex);
    [[nodiscard]] bool replaceSources(const QList<media::domain::MediaSource>& sources);
    [[nodiscard]] bool clearQueue();
    [[nodiscard]] bool reloadCurrentEntry();
    [[nodiscard]] bool nextEntry();
    [[nodiscard]] bool previousEntry();
    [[nodiscard]] bool setRepeatMode(domain::PlaylistRepeatMode mode);
    [[nodiscard]] bool setShuffleEnabled(bool enabled);

    Q_INVOKABLE bool removeEntry(quint64 entryId);
    Q_INVOKABLE bool moveEntry(quint64 entryId, int targetIndex);
    Q_INVOKABLE bool selectEntry(quint64 entryId);

signals:
    void playlistChanged();

private:
    [[nodiscard]] bool claimManualNavigation();
    void releaseManualNavigationClaim() noexcept;
    [[nodiscard]] bool submitLoadForEntry(const domain::PlaylistEntry& entry);
    void restoreCurrent(std::optional<domain::PlaylistEntryId> previousCurrent) noexcept;

    domain::Playlist& playlist_;
    PlaylistMutation& mutation_;
    SubmitMediaLoad submitMediaLoad_;
    SubmitMediaStop submitMediaStop_;
    PlaylistAdvanceArbiter* advanceArbiter_ = nullptr;
};

} // namespace player::playlist::application
