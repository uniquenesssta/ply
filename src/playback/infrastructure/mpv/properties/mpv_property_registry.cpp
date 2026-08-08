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
