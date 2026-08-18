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

class PlaylistEntryPlaybackState;

class PlaylistListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentPosition READ currentPosition NOTIFY currentPositionChanged)

public:
    enum Role {
        EntryIdRole = Qt::UserRole + 1,
        DisplayTitleRole,
        SourceLocationRole,
        SourceKindRole,
        CurrentRole,
        PendingLoadingRole,
        UnavailableRole,
    };
    Q_ENUM(Role)

    explicit PlaylistListModel(
        application::PlaylistController& controller,
        QObject* parent = nullptr);
    PlaylistListModel(
        application::PlaylistController& controller,
        PlaylistEntryPlaybackState& entryPlaybackState,
        QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] int count() const noexcept;
    [[nodiscard]] int currentPosition() const noexcept;

signals:
    void countChanged();
    void currentPositionChanged();

private slots:
    void refresh();

private:
    struct Row final {
        quint64 entryId = 0;
        QString displayTitle;
        QString sourceLocation;
        int sourceKind = 0;
        bool current = false;
        bool pendingLoading = false;
        bool unavailable = false;
    };

    [[nodiscard]] static QString displayTitleFor(const QString& location, int sourceKind);

    application::PlaylistController& controller_;
    PlaylistEntryPlaybackState* entryPlaybackState_ = nullptr;
    QVector<Row> rows_;
    int currentPosition_ = 0;
};

} // namespace player::playlist::presentation
