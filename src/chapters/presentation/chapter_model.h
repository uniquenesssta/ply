#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>
#include <QtGlobal>

namespace player::chapters::presentation {

class ChapterModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        IndexRole = Qt::UserRole + 1,
        TitleRole,
        TimeRole,
        TimeTextRole,
        NormalizedTimeRole,
    };
    Q_ENUM(Role)

    explicit ChapterModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] int count() const noexcept;

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void countChanged();

private:
    struct Row final
    {
        qsizetype index = 0;
        QString title;
        double timeSeconds = 0.0;
        QString timeText;
        double normalizedTime = 0.0;

        bool operator==(const Row&) const = default;
    };

    QVector<Row> rows_;
};

} // namespace player::chapters::presentation
