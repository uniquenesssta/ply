#include "tracks/presentation/chapter_list_model.h"

#include <utility>

namespace player::tracks::presentation {

using player::playback::domain::ChapterDescriptor;
using player::playback::domain::PlaybackSnapshot;

ChapterListModel::ChapterListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ChapterListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}

QVariant ChapterListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || static_cast<qsizetype>(index.row()) >= rows_.size()) {
        return {};
    }

    const Row& row = rows_.at(index.row());
    switch (role) {
    case ChapterIndexRole:
        return row.index;
    case TitleRole:
        return row.title;
    case StartSecondsRole:
        return row.startSeconds;
    default:
        return {};
    }
}

QHash<int, QByteArray> ChapterListModel::roleNames() const
{
    return {
        {ChapterIndexRole, QByteArrayLiteral("chapterIndex")},
        {TitleRole, QByteArrayLiteral("title")},
        {StartSecondsRole, QByteArrayLiteral("startSeconds")},
    };
}

Qt::ItemFlags ChapterListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int ChapterListModel::count() const noexcept
{
    return static_cast<int>(rows_.size());
}

void ChapterListModel::acceptSnapshot(const PlaybackSnapshot& snapshot)
{
    const qsizetype previousCount = rows_.size();

    QVector<Row> nextRows;
    nextRows.reserve(static_cast<qsizetype>(snapshot.chapters().chapters.size()));
    for (const ChapterDescriptor& chapter : snapshot.chapters().chapters) {
        nextRows.push_back(Row{
            static_cast<int>(chapter.index),
            displayTitleFor(chapter.title.value_or(QString{}), static_cast<int>(chapter.index)),
            chapter.startSeconds,
        });
    }

    beginResetModel();
    rows_ = std::move(nextRows);
    endResetModel();

    if (previousCount != rows_.size()) {
        emit countChanged();
    }
}

QString ChapterListModel::displayTitleFor(const QString& title, int index)
{
    if (!title.isEmpty()) {
        return title;
    }
    return QStringLiteral("Chapter %1").arg(index + 1);
}

} // namespace player::tracks::presentation
