#include "mpv_media_model_mapper.h"

#include "media_model_mapping/mpv_cache_status_mapper.h"
#include "media_model_mapping/mpv_chapter_model_mapper.h"
#include "media_model_mapping/mpv_media_model_value_reader.h"
#include "media_model_mapping/mpv_stream_info_mapper.h"
#include "media_model_mapping/mpv_track_model_mapper.h"

#include <QString>

namespace player::playback::mpv {

std::optional<player::playback::domain::PlaybackEvent> MpvMediaModelMapper::map(
    const MpvPropertyChange& change,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    switch (change.id) {
    case MpvPropertyId::TrackList:
        return MpvTrackModelMapper::mapTrackList(change.value, errorMessage);
    case MpvPropertyId::ChapterList:
        return MpvChapterModelMapper::map(change.value, errorMessage);
    case MpvPropertyId::SelectedVideoTrack:
    case MpvPropertyId::SelectedAudioTrack:
    case MpvPropertyId::SelectedSubtitleTrack:
        return MpvTrackModelMapper::mapSelection(change.id, change.value, errorMessage);
    case MpvPropertyId::VideoParams:
        return MpvStreamInfoMapper::mapVideo(change.value, errorMessage);
    case MpvPropertyId::AudioParams:
        return MpvStreamInfoMapper::mapAudio(change.value, errorMessage);
    case MpvPropertyId::DemuxerCacheState:
        return MpvCacheStatusMapper::map(change.value, errorMessage);
    default:
        MpvMediaModelValueReader::setError(
            errorMessage,
            QStringLiteral("Property id is not owned by MpvMediaModelMapper."));
        return std::nullopt;
    }
}

} // namespace player::playback::mpv
