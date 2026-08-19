#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

namespace player::playback::mpv {
namespace {

const QList<MpvPropertyDefinition> kCoreDefinitions{
    {MpvPropertyId::Position, 2001, QByteArrayLiteral("time-pos"), MpvPropertyFormat::Double},
    {MpvPropertyId::Duration, 2002, QByteArrayLiteral("duration"), MpvPropertyFormat::Double},
    {MpvPropertyId::Pause, 2003, QByteArrayLiteral("pause"), MpvPropertyFormat::Flag},
    {MpvPropertyId::Volume, 2004, QByteArrayLiteral("volume"), MpvPropertyFormat::Double},
    {MpvPropertyId::Mute, 2005, QByteArrayLiteral("mute"), MpvPropertyFormat::Flag},
    {MpvPropertyId::Speed, 2006, QByteArrayLiteral("speed"), MpvPropertyFormat::Double},
    {MpvPropertyId::Seekable, 2007, QByteArrayLiteral("seekable"), MpvPropertyFormat::Flag},
    {MpvPropertyId::CoreIdle, 2008, QByteArrayLiteral("core-idle"), MpvPropertyFormat::Flag},
    {MpvPropertyId::EofReached, 2009, QByteArrayLiteral("eof-reached"), MpvPropertyFormat::Flag},
    {MpvPropertyId::TrackList, 2010, QByteArrayLiteral("track-list"), MpvPropertyFormat::Node},
    {MpvPropertyId::ChapterList, 2011, QByteArrayLiteral("chapter-list"), MpvPropertyFormat::Node},
    {MpvPropertyId::Seeking, 2012, QByteArrayLiteral("seeking"), MpvPropertyFormat::Flag},
    {MpvPropertyId::PausedForCache, 2013, QByteArrayLiteral("paused-for-cache"), MpvPropertyFormat::Flag},
    {MpvPropertyId::CacheBufferingState, 2014, QByteArrayLiteral("cache-buffering-state"), MpvPropertyFormat::Node},
    {MpvPropertyId::DemuxerCacheState, 2015, QByteArrayLiteral("demuxer-cache-state"), MpvPropertyFormat::Node},
    {MpvPropertyId::MediaTitle, 2016, QByteArrayLiteral("media-title"), MpvPropertyFormat::String},
    {MpvPropertyId::Path, 2017, QByteArrayLiteral("path"), MpvPropertyFormat::String},
    {MpvPropertyId::SelectedAudioTrack, 2018, QByteArrayLiteral("aid"), MpvPropertyFormat::Node},
    {MpvPropertyId::SelectedSubtitleTrack, 2019, QByteArrayLiteral("sid"), MpvPropertyFormat::Node},
    {MpvPropertyId::SelectedVideoTrack, 2020, QByteArrayLiteral("vid"), MpvPropertyFormat::Node},
    {MpvPropertyId::VideoParams, 2021, QByteArrayLiteral("video-params"), MpvPropertyFormat::Node},
    {MpvPropertyId::AudioParams, 2022, QByteArrayLiteral("audio-params"), MpvPropertyFormat::Node},
    {MpvPropertyId::SubtitleDelay, 2023, QByteArrayLiteral("sub-delay"), MpvPropertyFormat::Double},
    {MpvPropertyId::AudioDelay, 2024, QByteArrayLiteral("audio-delay"), MpvPropertyFormat::Double},
};

} // namespace

const QList<MpvPropertyDefinition>& MpvPropertyRegistry::coreDefinitions()
{
    return kCoreDefinitions;
}

const MpvPropertyDefinition* MpvPropertyRegistry::findById(MpvPropertyId id) noexcept
{
    for (const MpvPropertyDefinition& definition : kCoreDefinitions) {
        if (definition.id == id) {
            return &definition;
        }
    }
    return nullptr;
}

const MpvPropertyDefinition* MpvPropertyRegistry::findByObservationId(quint64 observationId) noexcept
{
    for (const MpvPropertyDefinition& definition : kCoreDefinitions) {
        if (definition.observationId == observationId) {
            return &definition;
        }
    }
    return nullptr;
}

} // namespace player::playback::mpv
