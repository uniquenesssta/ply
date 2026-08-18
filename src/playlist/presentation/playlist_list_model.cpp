#include "playlist/presentation/playlist_list_model.h"

#include "media/domain/media_source.h"
#include "playlist/application/playlist_controller.h"
#include "playlist/domain/playlist_snapshot.h"
#include "playlist/presentation/playlist_entry_playback_state.h"

#include <QFileInfo>
#include <QUrl>

#include <utility>

namespace player::playlist::presentation {

PlaylistListModel::PlaylistListModel(
    application::PlaylistController& controller,
    QObject* parent)
    : QAbstractListModel(parent)
    , controller_(controller)
{
    connect(
        &controller_,
        &application::PlaylistController::playlistChanged,
        this,
        &PlaylistListModel::refresh);
    refresh();
}

PlaylistListModel::PlaylistListModel(
    application::PlaylistController& controller,
    PlaylistEntryPlaybackState& entryPlaybackState,
    QObject* parent)
    : PlaylistListModel(controller, parent)
{
    entryPlaybackState_ = &entryPlaybackState;
    connect(
        entryPlaybackState_,
        &PlaylistEntryPlaybackState::entryStatesChanged,
        this,
        &PlaylistListModel::refresh);
    refresh();
}

int PlaylistListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}

QVariant PlaylistListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || static_cast<qsizetype>(index.row()) >= rows_.size()) {
        return {};
    }

    const Row& row = rows_.at(index.row());
    switch (role) {
    case EntryIdRole:
        return QVariant::fromValue(row.entryId);
    case DisplayTitleRole:
        return row.displayTitle;
    case SourceLocationRole:
        return row.sourceLocation;
    case SourceKindRole:
        return row.sourceKind;
    case CurrentRole:
        return row.current;
    case PendingLoadingRole:
        return row.pendingLoading;
    case UnavailableRole:
        return row.unavailable;
    default:
        return {};
    }
}

QHash<int, QByteArray> PlaylistListModel::roleNames() const
{
    return {
        {EntryIdRole, QByteArrayLiteral("entryId")},
        {DisplayTitleRole, QByteArrayLiteral("displayTitle")},
        {SourceLocationRole, QByteArrayLiteral("sourceLocation")},
        {SourceKindRole, QByteArrayLiteral("sourceKind")},
        {CurrentRole, QByteArrayLiteral("current")},
        {PendingLoadingRole, QByteArrayLiteral("pendingLoading")},
        {UnavailableRole, QByteArrayLiteral("unavailable")},
    };
}

Qt::ItemFlags PlaylistListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int PlaylistListModel::count() const noexcept
{
    return static_cast<int>(rows_.size());
}

int PlaylistListModel::currentPosition() const noexcept
{
    return currentPosition_;
}

void PlaylistListModel::refresh()
{
    const qsizetype previousCount = rows_.size();
    const int previousCurrentPosition = currentPosition_;

    const domain::PlaylistSnapshot snapshot = controller_.snapshot();
    QVector<Row> nextRows;
    nextRows.reserve(static_cast<qsizetype>(snapshot.size()));

    const std::optional<std::size_t> currentIndex = snapshot.currentIndex();
    const int nextCurrentPosition = currentIndex.has_value()
        ? static_cast<int>(*currentIndex + 1)
        : 0;
    const std::optional<domain::PlaylistEntryId> currentId = snapshot.currentId();

    for (const domain::PlaylistEntry& entry : snapshot.entries()) {
        const bool current = currentId.has_value() && *currentId == entry.id();
        const int sourceKind = static_cast<int>(entry.source().kind());
        const bool pendingLoading = entryPlaybackState_ != nullptr
            && entryPlaybackState_->isPendingLoading(entry.id());
        const bool unavailable = entryPlaybackState_ != nullptr
            && entryPlaybackState_->isUnavailable(entry.id());
        nextRows.push_back(Row{
            entry.id().value(),
            displayTitleFor(entry.source().location(), sourceKind),
            entry.source().location(),
            sourceKind,
            current,
            pendingLoading,
            unavailable,
        });
    }

    beginResetModel();
    rows_ = std::move(nextRows);
    currentPosition_ = nextCurrentPosition;
    endResetModel();

    if (previousCount != rows_.size()) {
        emit countChanged();
    }
    if (previousCurrentPosition != currentPosition_) {
        emit currentPositionChanged();
    }
}

QString PlaylistListModel::displayTitleFor(const QString& location, int sourceKind)
{
    if (sourceKind == static_cast<int>(media::domain::MediaSourceKind::LocalFile)) {
        const QString fileName = QFileInfo(location).fileName();
        return fileName.isEmpty() ? location : fileName;
    }

    const QUrl url(location);
    if (!url.fileName().isEmpty()) {
        return url.fileName();
    }
    if (!url.host().isEmpty()) {
        return url.host();
    }
    return location;
}

} // namespace player::playlist::presentation
