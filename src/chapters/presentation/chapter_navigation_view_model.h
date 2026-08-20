#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QObject>
#include <QString>
#include <QVector>

#include <optional>

namespace player::chapters::presentation {

class ChapterNavigationViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSeek READ canSeek NOTIFY stateChanged)
    Q_PROPERTY(bool canSeekPrevious READ canSeekPrevious NOTIFY stateChanged)
    Q_PROPERTY(bool canSeekNext READ canSeekNext NOTIFY stateChanged)
    Q_PROPERTY(qint64 currentChapterIndex READ currentChapterIndex NOTIFY stateChanged)
    Q_PROPERTY(qint64 pendingChapterIndex READ pendingChapterIndex NOTIFY stateChanged)
    Q_PROPERTY(int currentPosition READ currentPosition NOTIFY stateChanged)
    Q_PROPERTY(int chapterCount READ chapterCount NOTIFY stateChanged)
    Q_PROPERTY(QString currentChapterTitle READ currentChapterTitle NOTIFY stateChanged)

public:
    explicit ChapterNavigationViewModel(QObject* parent = nullptr);

    [[nodiscard]] bool canSeek() const noexcept;
    [[nodiscard]] bool canSeekPrevious() const noexcept;
    [[nodiscard]] bool canSeekNext() const noexcept;
    [[nodiscard]] qint64 currentChapterIndex() const noexcept;
    [[nodiscard]] qint64 pendingChapterIndex() const noexcept;
    [[nodiscard]] int currentPosition() const noexcept;
    [[nodiscard]] int chapterCount() const noexcept;
    [[nodiscard]] QString currentChapterTitle() const;

    Q_INVOKABLE bool requestChapterSeek(qint64 chapterIndex);
    Q_INVOKABLE bool requestPreviousChapter();
    Q_INVOKABLE bool requestNextChapter();

    [[nodiscard]] bool rejectPendingSeek();

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();
    void seekRequested(double absoluteSeconds);

private:
    struct Row final
    {
        qint64 index = -1;
        QString title;
        double timeSeconds = 0.0;

        bool operator==(const Row&) const = default;
    };

    struct PendingSeek final
    {
        player::playback::domain::MediaGeneration generation;
        qint64 chapterIndex = -1;
        double timeSeconds = 0.0;
    };

    [[nodiscard]] bool requestRow(qsizetype row);
    [[nodiscard]] qsizetype rowForChapterIndex(qint64 chapterIndex) const noexcept;
    [[nodiscard]] static qsizetype currentRowForPosition(
        const QVector<Row>& rows,
        const std::optional<double>& positionSeconds) noexcept;
    [[nodiscard]] static QString displayTitle(
        const player::playback::domain::ChapterDescriptor& chapter);

    player::playback::domain::MediaGeneration generation_;
    QVector<Row> rows_;
    qsizetype currentRow_ = -1;
    bool canSeek_ = false;
    std::optional<PendingSeek> pendingSeek_;
};

} // namespace player::chapters::presentation
