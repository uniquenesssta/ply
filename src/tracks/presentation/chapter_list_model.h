#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>
#include <QtGlobal>

namespace player::tracks::presentation {

// Read-only projection of the current media chapter list from the
// authoritative PlaybackSnapshot. Chapter clicks produce a unified Seek
// action; this model never writes Timeline state.
class ChapterListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        ChapterIndexRole = Qt::UserRole + 1,
        TitleRole,
        StartSecondsRole,
    };
    Q_ENUM(Role)

    explicit ChapterListModel(QObject* parent = nullptr);

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
    struct Row final {
        int index = 0;
        QString title;
        double startSeconds = 0.0;
    };

    [[nodiscard]] static QString displayTitleFor(const QString& title, int index);

    QVector<Row> rows_;
};

} // namespace player::tracks::presentation
