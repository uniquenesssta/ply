#include "chapters/presentation/chapter_navigation_view_model.h"

#include "playback/domain/state/playback_selectors.h"

#include <cmath>
#include <limits>
#include <utility>

namespace player::chapters::presentation {
namespace {

constexpr double kSeekAcknowledgementToleranceSeconds = 0.75;

bool validPosition(const std::optional<double>& seconds) noexcept
{
    return seconds.has_value() && std::isfinite(*seconds) && *seconds >= 0.0;
}

} // namespace

ChapterNavigationViewModel::ChapterNavigationViewModel(QObject* parent)
    : QObject(parent)
{
}

bool ChapterNavigationViewModel::canSeek() const noexcept
{
    return canSeek_;
}

bool ChapterNavigationViewModel::canSeekPrevious() const noexcept
{
    return canSeek_ && currentRow_ > 0;
}

bool ChapterNavigationViewModel::canSeekNext() const noexcept
{
    return canSeek_
        && currentRow_ >= 0
        && currentRow_ + 1 < rows_.size();
}

qint64 ChapterNavigationViewModel::currentChapterIndex() const noexcept
{
    return currentRow_ >= 0 && currentRow_ < rows_.size()
        ? rows_.at(currentRow_).index
        : -1;
}

qint64 ChapterNavigationViewModel::pendingChapterIndex() const noexcept
{
    return pendingSeek_.has_value() ? pendingSeek_->chapterIndex : -1;
}

int ChapterNavigationViewModel::currentPosition() const noexcept
{
    return currentRow_ >= 0 ? static_cast<int>(currentRow_ + 1) : 0;
}

int ChapterNavigationViewModel::chapterCount() const noexcept
{
    return static_cast<int>(rows_.size());
}

QString ChapterNavigationViewModel::currentChapterTitle() const
{
    return currentRow_ >= 0 && currentRow_ < rows_.size()
        ? rows_.at(currentRow_).title
        : QString{};
}

bool ChapterNavigationViewModel::requestChapterSeek(qint64 chapterIndex)
{
    return requestRow(rowForChapterIndex(chapterIndex));
}

bool ChapterNavigationViewModel::requestPreviousChapter()
{
    if (!canSeekPrevious()) {
        return false;
    }
    return requestRow(currentRow_ - 1);
}

bool ChapterNavigationViewModel::requestNextChapter()
{
    if (!canSeekNext()) {
        return false;
    }
    return requestRow(currentRow_ + 1);
}

bool ChapterNavigationViewModel::rejectPendingSeek()
{
    if (!pendingSeek_.has_value()) {
        return false;
    }

    pendingSeek_.reset();
    emit stateChanged();
    return true;
}

void ChapterNavigationViewModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const bool previousCanSeek = canSeek();
    const bool previousCanSeekPrevious = canSeekPrevious();
    const bool previousCanSeekNext = canSeekNext();
    const qint64 previousCurrentIndex = currentChapterIndex();
    const qint64 previousPendingIndex = pendingChapterIndex();
    const int previousCurrentPosition = currentPosition();
    const int previousChapterCount = chapterCount();
    const QString previousCurrentTitle = currentChapterTitle();

    const auto nextGeneration = snapshot.generation();
    if (nextGeneration != generation_) {
        pendingSeek_.reset();
    }
    generation_ = nextGeneration;

    QVector<Row> nextRows;
    nextRows.reserve(snapshot.chapters().chapters.size());
    for (const auto& chapter : snapshot.chapters().chapters) {
        nextRows.push_back(Row{
            static_cast<qint64>(chapter.index),
            displayTitle(chapter),
            chapter.startSeconds,
        });
    }
    rows_ = std::move(nextRows);

    const auto positionSeconds = snapshot.timeline().positionSeconds;
    currentRow_ = currentRowForPosition(rows_, positionSeconds);
    canSeek_ = player::playback::domain::selectors::canSeek(snapshot)
        && generation_.isValid()
        && !rows_.isEmpty();

    if (!canSeek_
        || (pendingSeek_.has_value()
            && rowForChapterIndex(pendingSeek_->chapterIndex) < 0)) {
        pendingSeek_.reset();
    } else if (pendingSeek_.has_value()
               && pendingSeek_->generation == generation_
               && validPosition(positionSeconds)
               && std::abs(*positionSeconds - pendingSeek_->timeSeconds)
                   <= kSeekAcknowledgementToleranceSeconds) {
        pendingSeek_.reset();
    }

    const bool changed = previousCanSeek != canSeek()
        || previousCanSeekPrevious != canSeekPrevious()
        || previousCanSeekNext != canSeekNext()
        || previousCurrentIndex != currentChapterIndex()
        || previousPendingIndex != pendingChapterIndex()
        || previousCurrentPosition != currentPosition()
        || previousChapterCount != chapterCount()
        || previousCurrentTitle != currentChapterTitle();
    if (changed) {
        emit stateChanged();
    }
}

bool ChapterNavigationViewModel::requestRow(qsizetype row)
{
    if (!canSeek_
        || !generation_.isValid()
        || row < 0
        || row >= rows_.size()) {
        return false;
    }

    const Row& target = rows_.at(row);
    if (!std::isfinite(target.timeSeconds) || target.timeSeconds < 0.0) {
        return false;
    }

    pendingSeek_ = PendingSeek{
        generation_,
        target.index,
        target.timeSeconds,
    };
    emit stateChanged();
    emit seekRequested(target.timeSeconds);
    return true;
}

qsizetype ChapterNavigationViewModel::rowForChapterIndex(
    qint64 chapterIndex) const noexcept
{
    for (qsizetype row = 0; row < rows_.size(); ++row) {
        if (rows_.at(row).index == chapterIndex) {
            return row;
        }
    }
    return -1;
}

qsizetype ChapterNavigationViewModel::currentRowForPosition(
    const QVector<Row>& rows,
    const std::optional<double>& positionSeconds) noexcept
{
    if (!validPosition(positionSeconds)) {
        return -1;
    }

    qsizetype currentRow = -1;
    double currentStart = -std::numeric_limits<double>::infinity();
    for (qsizetype row = 0; row < rows.size(); ++row) {
        const double start = rows.at(row).timeSeconds;
        if (!std::isfinite(start) || start < 0.0 || start > *positionSeconds) {
            continue;
        }
        if (start >= currentStart) {
            currentStart = start;
            currentRow = row;
        }
    }
    return currentRow;
}

QString ChapterNavigationViewModel::displayTitle(
    const player::playback::domain::ChapterDescriptor& chapter)
{
    if (chapter.title.has_value() && !chapter.title->isEmpty()) {
        return *chapter.title;
    }
    return QStringLiteral("Chapter %1").arg(static_cast<qlonglong>(chapter.index + 1));
}

} // namespace player::chapters::presentation
