#include "chapters/presentation/chapter_model.h"

#include <QChar>
#include <QVariant>

#include <cmath>
#include <utility>

namespace player::chapters::presentation {
namespace {

QString displayTitle(const player::playback::domain::ChapterDescriptor& chapter)
{
    if (chapter.title.has_value() && !chapter.title->isEmpty()) {
        return *chapter.title;
    }
    return QStringLiteral("Chapter %1").arg(static_cast<qlonglong>(chapter.index + 1));
}

QString displayTime(double seconds)
{
    if (!std::isfinite(seconds) || seconds < 0.0) {
        return QStringLiteral("--:--:--");
    }

    const qint64 totalSeconds = static_cast<qint64>(std::floor(seconds));
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 remainingSeconds = totalSeconds % 60;
    return QStringLiteral("%1:%2:%3")
        .arg(hours, 2, 10, QChar(u'0'))
        .arg(minutes, 2, 10, QChar(u'0'))
        .arg(remainingSeconds, 2, 10, QChar(u'0'));
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
    case TimeTextRole:
        return row.timeText;
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
            displayTime(chapter.startSeconds),
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
