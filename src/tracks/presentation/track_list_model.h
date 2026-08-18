#pragma once

#include "playback/domain/models/track_descriptor.h"
#include "playback/domain/state/playback_snapshot.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>
#include <QtGlobal>

namespace player::tracks::presentation {

class TrackListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(qint64 selectedTrackId READ selectedTrackId NOTIFY selectedTrackIdChanged)

public:
    enum Role {
        TrackIdRole = Qt::UserRole + 1,
        TitleRole,
        LanguageRole,
        CodecRole,
        SelectedRole,
        DefaultRole,
        ForcedRole,
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
    [[nodiscard]] qint64 selectedTrackId() const noexcept;
    [[nodiscard]] player::playback::domain::TrackKind kind() const noexcept;

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void countChanged();
    void selectedTrackIdChanged();

private:
    struct Row final
    {
        qint64 trackId = 0;
        QString title;
        QString language;
        QString codec;
        bool selected = false;
        bool defaultTrack = false;
        bool forced = false;
        bool external = false;
        QString externalFilename;

        bool operator==(const Row&) const = default;
    };

    player::playback::domain::TrackKind kind_;
    QVector<Row> rows_;
    qint64 selectedTrackId_ = 0;
};

} // namespace player::tracks::presentation
