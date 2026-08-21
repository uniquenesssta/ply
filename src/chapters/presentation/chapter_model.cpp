#include "chapters/presentation/chapter_model.h"

#include "chapters/presentation/chapter_display_formatter.h"

#include <QVariant>

#include <cmath>
#include <utility>

namespace player::chapters::presentation {

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
    case TimeTextRole:
        return row.timeText;
    case NormalizedTimeRole:
        return row.normalizedTime;
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
        {TimeTextRole, QByteArrayLiteral("timeText")},
        {NormalizedTimeRole, QByteArrayLiteral("normalizedTime")},
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
    const auto durationSeconds = snapshot.timeline().durationSeconds;
    const bool hasValidDuration = durationSeconds.has_value()
        && std::isfinite(*durationSeconds)
        && *durationSeconds > 0.0;
    const auto chapters = snapshot.chapters().validatedForDuration(durationSeconds);

    QVector<Row> nextRows;
    nextRows.reserve(chapters.size());
    for (const player::playback::domain::ChapterDescriptor& chapter : chapters) {
        nextRows.push_back(Row{
            chapter.index,
            ChapterDisplayFormatter::title(chapter),
            chapter.startSeconds,
            ChapterDisplayFormatter::time(chapter.startSeconds),
            hasValidDuration ? chapter.startSeconds / *durationSeconds : 0.0,
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

} // namespace player::chapters::presentation
