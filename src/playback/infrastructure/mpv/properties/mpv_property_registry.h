#pragma once

#include <QByteArray>
#include <QList>
#include <QtGlobal>

namespace player::playback::mpv {

enum class MpvPropertyId : quint16
{
    Position,
    Duration,
    Pause,
    Volume,
    Mute,
    Speed,
    Seekable,
    CoreIdle,
    EofReached,
    TrackList,
    ChapterList,
    Seeking,
    PausedForCache,
    CacheBufferingState,
    DemuxerCacheState,
    MediaTitle,
    Path,
    SelectedAudioTrack,
    SelectedSubtitleTrack,
    SelectedVideoTrack,
    VideoParams,
    AudioParams,
};

enum class MpvPropertyFormat : quint8
{
    Flag,
    Double,
    String,
    Node,
};

struct MpvPropertyDefinition final
{
    MpvPropertyId id;
    quint64 observationId;
    QByteArray name;
    MpvPropertyFormat format;
};

class MpvPropertyRegistry final
{
public:
    [[nodiscard]] static const QList<MpvPropertyDefinition>& coreDefinitions();
    [[nodiscard]] static const MpvPropertyDefinition* findById(MpvPropertyId id) noexcept;
    [[nodiscard]] static const MpvPropertyDefinition* findByObservationId(quint64 observationId) noexcept;
};

} // namespace player::playback::mpv
