#include "presentation/viewmodels/player/media/player_media_view_model.h"

#include <QFileInfo>
#include <QStringList>
#include <QUrl>

namespace player::presentation {
namespace {

bool lifecycleHasEstablishedMedia(
    player::playback::domain::PlaybackLifecycleState lifecycle) noexcept
{
    using player::playback::domain::PlaybackLifecycleState;
    return lifecycle == PlaybackLifecycleState::Ready
        || lifecycle == PlaybackLifecycleState::Ended;
}

QString titleFor(const player::playback::domain::PlaybackMediaState& media)
{
    if (media.title.has_value() && !media.title->trimmed().isEmpty()) {
        return media.title->trimmed();
    }

    const auto candidate = media.path.has_value() && !media.path->trimmed().isEmpty()
        ? media.path
        : media.source;
    if (!candidate.has_value() || candidate->trimmed().isEmpty()) {
        return {};
    }

    const QString source = candidate->trimmed();
    const QUrl url(source);
    QString fileName;
    if (url.isLocalFile()) {
        fileName = QFileInfo(url.toLocalFile()).fileName();
    } else if (url.isValid() && !url.fileName().isEmpty()) {
        fileName = url.fileName();
    } else {
        fileName = QFileInfo(source).fileName();
    }
    return fileName.isEmpty() ? source : fileName;
}

QString metadataFor(const player::playback::domain::PlaybackStreamState& streams)
{
    if (!streams.video.has_value()) {
        return {};
    }

    const auto& video = *streams.video;
    const auto width = video.displayWidth.has_value() ? video.displayWidth : video.width;
    const auto height = video.displayHeight.has_value() ? video.displayHeight : video.height;

    QStringList parts;
    if (width.has_value() && height.has_value() && *width > 0 && *height > 0) {
        parts.append(QStringLiteral("%1×%2").arg(*width).arg(*height));
    }
    if (video.pixelFormat.has_value() && !video.pixelFormat->trimmed().isEmpty()) {
        parts.append(video.pixelFormat->trimmed());
    }
    return parts.join(QStringLiteral(" · "));
}

} // namespace

PlayerMediaViewModel::PlayerMediaViewModel(QObject* parent)
    : QObject(parent)
{
}

bool PlayerMediaViewModel::hasMedia() const noexcept
{
    return hasMedia_;
}

bool PlayerMediaViewModel::hasVideo() const noexcept
{
    return hasVideo_;
}

QString PlayerMediaViewModel::title() const
{
    return title_;
}

QString PlayerMediaViewModel::metadataText() const
{
    return metadataText_;
}

void PlayerMediaViewModel::acceptSnapshot(
    const player::playback::domain::PlaybackSnapshot& snapshot)
{
    const auto& source = snapshot.media().source;
    const bool nextHasMedia = lifecycleHasEstablishedMedia(snapshot.lifecycle())
        && source.has_value()
        && !source->isEmpty();
    const bool nextHasVideo = nextHasMedia && snapshot.capabilities().hasVideoTrack;
    const QString nextTitle = titleFor(snapshot.media());
    const QString nextMetadataText = metadataFor(snapshot.streams());

    if (nextHasMedia == hasMedia_
        && nextHasVideo == hasVideo_
        && nextTitle == title_
        && nextMetadataText == metadataText_) {
        return;
    }

    hasMedia_ = nextHasMedia;
    hasVideo_ = nextHasVideo;
    title_ = nextTitle;
    metadataText_ = nextMetadataText;
    emit stateChanged();
}

} // namespace player::presentation
