#include "playlist/presentation/playlist_list_model.h"

#include "media/domain/media_source.h"
#include "playlist/application/playlist_controller.h"

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

void PlaylistListModel::refresh()
{
    const qsizetype previousCount = rows_.size();

    QVector<Row> nextRows;
    const domain::Playlist& playlist = controller_.playlist();
    nextRows.reserve(static_cast<qsizetype>(playlist.size()));

    const std::optional<domain::PlaylistEntryId> currentId = playlist.currentId();
    for (const domain::PlaylistEntry& entry : playlist.entries()) {
        const int sourceKind = static_cast<int>(entry.source().kind());
        nextRows.push_back(Row{
            entry.id().value(),
            displayTitleFor(entry.source().location(), sourceKind),
            entry.source().location(),
            sourceKind,
            currentId.has_value() && *currentId == entry.id(),
        });
    }

    beginResetModel();
    rows_ = std::move(nextRows);
    endResetModel();

    if (previousCount != rows_.size()) {
        emit countChanged();
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
