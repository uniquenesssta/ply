#include "tracks/presentation/track_list_model.h"

#include <QFileInfo>

#include <utility>

namespace player::tracks::presentation {

using player::playback::domain::PlaybackSnapshot;
using player::playback::domain::TrackDescriptor;
using player::playback::domain::TrackKind;

TrackListModel::TrackListModel(TrackKind kind, QObject* parent)
    : QAbstractListModel(parent)
    , kind_(kind)
{
}

int TrackListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}

QVariant TrackListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || static_cast<qsizetype>(index.row()) >= rows_.size()) {
        return {};
    }

    const Row& row = rows_.at(index.row());
    switch (role) {
    case TrackIdRole:
        return QVariant::fromValue(row.trackId);
    case TitleRole:
        return row.title;
    case LanguageRole:
        return row.language;
    case CodecRole:
        return row.codec;
    case SelectedRole:
        return row.selected;
    case ExternalRole:
        return row.external;
    case ExternalFilenameRole:
        return row.externalFilename;
    default:
        return {};
    }
}

QHash<int, QByteArray> TrackListModel::roleNames() const
{
    return {
        {TrackIdRole, QByteArrayLiteral("trackId")},
        {TitleRole, QByteArrayLiteral("title")},
        {LanguageRole, QByteArrayLiteral("language")},
        {CodecRole, QByteArrayLiteral("codec")},
        {SelectedRole, QByteArrayLiteral("selected")},
        {ExternalRole, QByteArrayLiteral("external")},
        {ExternalFilenameRole, QByteArrayLiteral("externalFilename")},
    };
}

Qt::ItemFlags TrackListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int TrackListModel::count() const noexcept
{
    return static_cast<int>(rows_.size());
}

int TrackListModel::selectedRow() const noexcept
{
    return selectedRow_;
}

TrackKind TrackListModel::kind() const noexcept
{
    return kind_;
}

void TrackListModel::acceptSnapshot(const PlaybackSnapshot& snapshot)
{
    const qsizetype previousCount = rows_.size();
    const int previousSelectedRow = selectedRow_;

    QVector<Row> nextRows;
    int nextSelectedRow = -1;
    for (const TrackDescriptor& track : snapshot.tracks().tracks) {
        if (track.kind != kind_) {
            continue;
        }
        nextRows.push_back(Row{
            track.id,
            displayTitleFor(
                track.title.value_or(QString{}),
                track.language.value_or(QString{}),
                track.codec.value_or(QString{}),
                track.externalFilename.value_or(QString{})),
            track.language.value_or(QString{}),
            track.codec.value_or(QString{}),
            track.selected,
            track.external,
            track.externalFilename.value_or(QString{}),
        });
        if (track.selected && nextSelectedRow < 0) {
            nextSelectedRow = static_cast<int>(nextRows.size() - 1);
        }
    }

    beginResetModel();
    rows_ = std::move(nextRows);
    selectedRow_ = nextSelectedRow;
    endResetModel();

    if (previousCount != rows_.size()) {
        emit countChanged();
    }
    if (previousSelectedRow != selectedRow_) {
        emit selectedRowChanged();
    }
}

QString TrackListModel::displayTitleFor(
    const QString& title,
    const QString& language,
    const QString& codec,
    const QString& externalFilename)
{
    if (!title.isEmpty()) {
        return title;
    }
    if (!language.isEmpty()) {
        return language;
    }
    if (!codec.isEmpty()) {
        return codec;
    }
    if (!externalFilename.isEmpty()) {
        const QString fileName = QFileInfo(externalFilename).fileName();
        return fileName.isEmpty() ? externalFilename : fileName;
    }
    return QStringLiteral("Track");
}

} // namespace player::tracks::presentation
