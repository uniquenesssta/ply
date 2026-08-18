#pragma once

#include "playback/domain/models/track_descriptor.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>
#include <QtGlobal>

namespace player::tracks::presentation {

// Read-only projection of one track family (audio/subtitle/video) from the
// authoritative PlaybackSnapshot. The model never owns queue/selection state;
// row identity is the backend stable track id, never a visual index.
class TrackListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int selectedRow READ selectedRow NOTIFY selectedRowChanged)

public:
    enum Role {
        TrackIdRole = Qt::UserRole + 1,
        TitleRole,
        LanguageRole,
        CodecRole,
        SelectedRole,
        ExternalRole,
        ExternalFilenameRole,
    };
    Q_ENUM(Role)

    explicit TrackListModel(
        player::playback::domain::TrackKind kind,
        QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] int count() const noexcept;
    [[nodiscard]] int selectedRow() const noexcept;
    [[nodiscard]] player::playback::domain::TrackKind kind() const noexcept;

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void countChanged();
    void selectedRowChanged();

private:
    struct Row final {
        qint64 trackId = 0;
        QString title;
        QString language;
        QString codec;
        bool selected = false;
        bool external = false;
        QString externalFilename;
    };

    [[nodiscard]] static QString displayTitleFor(
        const QString& title,
        const QString& language,
        const QString& codec,
        const QString& externalFilename);

    player::playback::domain::TrackKind kind_;
    QVector<Row> rows_;
    int selectedRow_ = -1;
};

} // namespace player::tracks::presentation
