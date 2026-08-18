#include "tracks/presentation/track_list_model.h"

#include "playback/domain/state/playback_track_state.h"

#include <QVariant>

#include <optional>
#include <utility>

namespace player::tracks::presentation {
namespace {

using player::playback::domain::PlaybackTrackState;
using player::playback::domain::TrackKind;

std::optional<qint64> selectedTrackIdFor(
    const PlaybackTrackState& state,
    TrackKind kind) noexcept
{
    switch (kind) {
    case TrackKind::Video:
        return state.selectedVideoId;
    case TrackKind::Audio:
        return state.selectedAudioId;
    case TrackKind::Subtitle:
        return state.selectedSubtitleId;
    }
    return std::nullopt;
}

QString optionalText(const std::optional<QString>& value)
{
    return value.has_value() ? *value : QString{};
}

} // namespace

TrackListModel::TrackListModel(
    player::playback::domain::TrackKind kind,
    QObject* parent)
    : QAbstractListModel(parent)
    , kind_(kind)
{
}

int TrackListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}

QVariant TrackListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || static_cast<qsizetype>(index.row()) >= rows_.size()) {
        return {};
    }

    const Row& row = rows_.at(index.row());
    switch (role) {
    case TrackIdRole:
        return QVariant::fromValue(row.trackId);
    case TitleRole:
        return row.title;
    case LanguageRole:
        return row.language;
    case CodecRole:
        return row.codec;
    case SelectedRole:
        return row.selected;
    case DefaultRole:
        return row.defaultTrack;
    case ForcedRole:
        return row.forced;
    case ExternalRole:
        return row.external;
    case ExternalFilenameRole:
        return row.externalFilename;
    default:
        return {};
    }
}

QHash<int, QByteArray> TrackListModel::roleNames() const
{
    return {
        {TrackIdRole, QByteArrayLiteral("trackId")},
        {TitleRole, QByteArrayLiteral("title")},
        {LanguageRole, QByteArrayLiteral("language")},
        {CodecRole, QByteArrayLiteral("codec")},
        {SelectedRole, QByteArrayLiteral("selected")},
        {DefaultRole, QByteArrayLiteral("defaultTrack")},
        {ForcedRole, QByteArrayLiteral("forced")},
        {ExternalRole, QByteArrayLiteral("external")},
        {ExternalFilenameRole, QByteArrayLiteral("externalFilename")},
    };
}

Qt::ItemFlags TrackListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int TrackListModel::count() const noexcept
{
    return static_cast<int>(rows_.size());
}

qint64 TrackListModel::selectedTrackId() const noexcept
{
    return selectedTrackId_;
}

player::playback::domain::TrackKind TrackListModel::kind() const noexcept
{
    return kind_;
}

void TrackListModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const auto& trackState = snapshot.tracks();
    const std::optional<qint64> selectedTrackId = selectedTrackIdFor(trackState, kind_);

    bool selectedTrackExists = false;
    QVector<Row> nextRows;
    nextRows.reserve(trackState.tracks.size());
    for (const player::playback::domain::TrackDescriptor& track : trackState.tracks) {
        if (track.kind != kind_) {
            continue;
        }

        const bool selected = selectedTrackId.has_value() && *selectedTrackId == track.id;
        selectedTrackExists = selectedTrackExists || selected;
        nextRows.push_back(Row{
            track.id,
            optionalText(track.title),
            optionalText(track.language),
            optionalText(track.codec),
            selected,
            track.defaultTrack,
            track.forced,
            track.external,
            optionalText(track.externalFilename),
        });
    }

    const qint64 nextSelectedTrackId = selectedTrackExists && selectedTrackId.has_value()
        ? *selectedTrackId
        : qint64{0};
    if (nextRows == rows_ && nextSelectedTrackId == selectedTrackId_) {
        return;
    }

    const qsizetype previousCount = rows_.size();
    const qint64 previousSelectedTrackId = selectedTrackId_;
    beginResetModel();
    rows_ = std::move(nextRows);
    selectedTrackId_ = nextSelectedTrackId;
    endResetModel();

    if (previousCount != rows_.size()) {
        emit countChanged();
    }
    if (previousSelectedTrackId != selectedTrackId_) {
        emit selectedTrackIdChanged();
    }
}

} // namespace player::tracks::presentation
