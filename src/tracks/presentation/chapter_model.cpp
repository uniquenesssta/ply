#include "tracks/presentation/chapter_model.h"

#include <QVariant>

#include <utility>

namespace player::tracks::presentation {
namespace {

QString displayTitle(const player::playback::domain::ChapterDescriptor& chapter)
{
    if (chapter.title.has_value() && !chapter.title->isEmpty()) {
        return *chapter.title;
    }
    return QStringLiteral("Chapter %1").arg(static_cast<qlonglong>(chapter.index + 1));
}

} // namespace

ChapterModel::ChapterModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ChapterModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}

QVariant ChapterModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || static_cast<qsizetype>(index.row()) >= rows_.size()) {
        return {};
    }

    const Row& row = rows_.at(index.row());
    switch (role) {
    case IndexRole:
        return QVariant::fromValue(static_cast<qlonglong>(row.index));
    case TitleRole:
        return row.title;
    case TimeRole:
        return row.timeSeconds;
    default:
        return {};
    }
}

QHash<int, QByteArray> ChapterModel::roleNames() const
{
    return {
        {IndexRole, QByteArrayLiteral("index")},
        {TitleRole, QByteArrayLiteral("title")},
        {TimeRole, QByteArrayLiteral("time")},
    };
}

Qt::ItemFlags ChapterModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int ChapterModel::count() const noexcept
{
    return static_cast<int>(rows_.size());
}

void ChapterModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const auto& chapterState = snapshot.chapters();

    QVector<Row> nextRows;
    nextRows.reserve(chapterState.chapters.size());
    for (const player::playback::domain::ChapterDescriptor& chapter : chapterState.chapters) {
        nextRows.push_back(Row{
            chapter.index,
            displayTitle(chapter),
            chapter.startSeconds,
        });
    }

    if (nextRows == rows_) {
        return;
    }

    const qsizetype previousCount = rows_.size();
    beginResetModel();
    rows_ = std::move(nextRows);
    endResetModel();

    if (previousCount != rows_.size()) {
        emit countChanged();
    }
}

} // namespace player::tracks::presentation
