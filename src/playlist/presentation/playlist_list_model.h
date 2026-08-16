#pragma once

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>
#include <QtGlobal>

namespace player::playlist::application {
class PlaylistController;
}

namespace player::playlist::presentation {

class PlaylistListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        EntryIdRole = Qt::UserRole + 1,
        DisplayTitleRole,
        SourceLocationRole,
        SourceKindRole,
        CurrentRole,
    };
    Q_ENUM(Role)

    explicit PlaylistListModel(
        application::PlaylistController& controller,
        QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] int count() const noexcept;

signals:
    void countChanged();

private slots:
    void refresh();

private:
    struct Row final {
        quint64 entryId = 0;
        QString displayTitle;
        QString sourceLocation;
        int sourceKind = 0;
        bool current = false;
    };

    [[nodiscard]] static QString displayTitleFor(const QString& location, int sourceKind);

    application::PlaylistController& controller_;
    QVector<Row> rows_;
};

} // namespace player::playlist::presentation
