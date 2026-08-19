#include "playback/infrastructure/mpv/properties/mpv_property_observer.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <mpv/client.h>

#include <QString>
#include <QVariantMap>
#include <QtTest>

#include <variant>

namespace player::playback::mpv {
namespace {

struct ExpectedProperty final
{
    MpvPropertyId id;
    quint64 observationId;
    const char* name;
    MpvPropertyFormat format;
};

const ExpectedProperty kExpectedProperties[]{
    {MpvPropertyId::Position, 2001, "time-pos", MpvPropertyFormat::Double},
    {MpvPropertyId::Duration, 2002, "duration", MpvPropertyFormat::Double},
    {MpvPropertyId::Pause, 2003, "pause", MpvPropertyFormat::Flag},
    {MpvPropertyId::Volume, 2004, "volume", MpvPropertyFormat::Double},
    {MpvPropertyId::Mute, 2005, "mute", MpvPropertyFormat::Flag},
    {MpvPropertyId::Speed, 2006, "speed", MpvPropertyFormat::Double},
    {MpvPropertyId::Seekable, 2007, "seekable", MpvPropertyFormat::Flag},
    {MpvPropertyId::CoreIdle, 2008, "core-idle", MpvPropertyFormat::Flag},
    {MpvPropertyId::EofReached, 2009, "eof-reached", MpvPropertyFormat::Flag},
    {MpvPropertyId::TrackList, 2010, "track-list", MpvPropertyFormat::Node},
    {MpvPropertyId::ChapterList, 2011, "chapter-list", MpvPropertyFormat::Node},
    {MpvPropertyId::Seeking, 2012, "seeking", MpvPropertyFormat::Flag},
    {MpvPropertyId::PausedForCache, 2013, "paused-for-cache", MpvPropertyFormat::Flag},
    {MpvPropertyId::CacheBufferingState, 2014, "cache-buffering-state", MpvPropertyFormat::Node},
    {MpvPropertyId::DemuxerCacheState, 2015, "demuxer-cache-state", MpvPropertyFormat::Node},
    {MpvPropertyId::MediaTitle, 2016, "media-title", MpvPropertyFormat::String},
    {MpvPropertyId::Path, 2017, "path", MpvPropertyFormat::String},
    {MpvPropertyId::SelectedAudioTrack, 2018, "aid", MpvPropertyFormat::Node},
    {MpvPropertyId::SelectedSubtitleTrack, 2019, "sid", MpvPropertyFormat::Node},
    {MpvPropertyId::SelectedVideoTrack, 2020, "vid", MpvPropertyFormat::Node},
    {MpvPropertyId::VideoParams, 2021, "video-params", MpvPropertyFormat::Node},
    {MpvPropertyId::AudioParams, 2022, "audio-params", MpvPropertyFormat::Node},
    {MpvPropertyId::SubtitleDelay, 2023, "sub-delay", MpvPropertyFormat::Double},
    {MpvPropertyId::AudioDelay, 2024, "audio-delay", MpvPropertyFormat::Double},
};

const MpvPropertyDefinition& requireDefinition(MpvPropertyId id)
{
    const MpvPropertyDefinition* definition = MpvPropertyRegistry::findById(id);
    Q_ASSERT(definition != nullptr);
    return *definition;
}

QVariant decodeNodeProperty(
    MpvPropertyId id,
    mpv_node& node,
    QString* errorMessage)
{
    const MpvPropertyDefinition& definition = requireDefinition(id);
    const auto change = MpvPropertyObserver::decode(
        definition.observationId,
        MPV_FORMAT_NODE,
        &node,
        errorMessage);
    if (!change.has_value()) {
        return {};
    }

    const QVariant* value = std::get_if<QVariant>(&change->value);
    return value == nullptr ? QVariant{} : *value;
}

} // namespace

class MpvPropertyBaselineTest final : public QObject
{
    Q_OBJECT

private slots:
    void registryContainsR2Baseline();
    void stringPropertiesAreOwnedQtValues();
    void selectedTracksPreserveNativeValueShape();
    void cacheAndStreamParametersUseDeepCopiedNodes();
};

void MpvPropertyBaselineTest::registryContainsR2Baseline()
{
    const QList<MpvPropertyDefinition>& definitions = MpvPropertyRegistry::coreDefinitions();
    QCOMPARE(definitions.size(), qsizetype{24});

    for (const ExpectedProperty& expected : kExpectedProperties) {
        const MpvPropertyDefinition* definition = MpvPropertyRegistry::findById(expected.id);
        QVERIFY(definition != nullptr);
        QCOMPARE(definition->observationId, expected.observationId);
        QCOMPARE(definition->name, QByteArray(expected.name));
        QCOMPARE(definition->format, expected.format);

        const MpvPropertyDefinition* byObservation =
            MpvPropertyRegistry::findByObservationId(expected.observationId);
        QVERIFY(byObservation != nullptr);
        QCOMPARE(byObservation->id, expected.id);
    }
}

void MpvPropertyBaselineTest::stringPropertiesAreOwnedQtValues()
{
    QString error;
    const MpvPropertyDefinition& title = requireDefinition(MpvPropertyId::MediaTitle);

    char titleText[] = "R2 property baseline";
    char* titleStorage = titleText;
    const auto titleChange = MpvPropertyObserver::decode(
        title.observationId,
        MPV_FORMAT_STRING,
        &titleStorage,
        &error);
    QVERIFY2(titleChange.has_value(), qPrintable(error));
    const QString* titleValue = std::get_if<QString>(&titleChange->value);
    QVERIFY(titleValue != nullptr);
    QCOMPARE(*titleValue, QStringLiteral("R2 property baseline"));

    titleText[0] = 'X';
    QCOMPARE(*titleValue, QStringLiteral("R2 property baseline"));

    const MpvPropertyDefinition& path = requireDefinition(MpvPropertyId::Path);
    char* nullString = nullptr;
    const auto unavailablePath = MpvPropertyObserver::decode(
        path.observationId,
        MPV_FORMAT_STRING,
        &nullString,
        &error);
    QVERIFY2(unavailablePath.has_value(), qPrintable(error));
    QVERIFY(std::holds_alternative<std::monostate>(unavailablePath->value));
}

void MpvPropertyBaselineTest::selectedTracksPreserveNativeValueShape()
{
    QString error;

    mpv_node audioTrack{};
    audioTrack.format = MPV_FORMAT_INT64;
    audioTrack.u.int64 = 7;
    const QVariant audioValue = decodeNodeProperty(
        MpvPropertyId::SelectedAudioTrack,
        audioTrack,
        &error);
    QVERIFY2(audioValue.isValid(), qPrintable(error));
    QCOMPARE(audioValue.toLongLong(), qlonglong{7});

    char noTrack[] = "no";
    mpv_node subtitleTrack{};
    subtitleTrack.format = MPV_FORMAT_STRING;
    subtitleTrack.u.string = noTrack;
    const QVariant subtitleValue = decodeNodeProperty(
        MpvPropertyId::SelectedSubtitleTrack,
        subtitleTrack,
        &error);
    QVERIFY2(subtitleValue.isValid(), qPrintable(error));
    QCOMPARE(subtitleValue.toString(), QStringLiteral("no"));
}

void MpvPropertyBaselineTest::cacheAndStreamParametersUseDeepCopiedNodes()
{
    QString error;

    mpv_node bufferingState{};
    bufferingState.format = MPV_FORMAT_INT64;
    bufferingState.u.int64 = 73;
    const QVariant bufferingValue = decodeNodeProperty(
        MpvPropertyId::CacheBufferingState,
        bufferingState,
        &error);
    QVERIFY2(bufferingValue.isValid(), qPrintable(error));
    QCOMPARE(bufferingValue.toLongLong(), qlonglong{73});

    char cacheKey[] = "cache-duration";
    char* cacheKeys[]{cacheKey};
    mpv_node cacheField{};
    cacheField.format = MPV_FORMAT_DOUBLE;
    cacheField.u.double_ = 4.5;
    mpv_node_list cacheMap{};
    cacheMap.num = 1;
    cacheMap.values = &cacheField;
    cacheMap.keys = cacheKeys;
    mpv_node cacheRoot{};
    cacheRoot.format = MPV_FORMAT_NODE_MAP;
    cacheRoot.u.list = &cacheMap;

    const QVariant cacheValue = decodeNodeProperty(
        MpvPropertyId::DemuxerCacheState,
        cacheRoot,
        &error);
    QVERIFY2(cacheValue.isValid(), qPrintable(error));
    QCOMPARE(cacheValue.toMap().value(QStringLiteral("cache-duration")).toDouble(), 4.5);

    char videoKey[] = "w";
    char* videoKeys[]{videoKey};
    mpv_node videoField{};
    videoField.format = MPV_FORMAT_INT64;
    videoField.u.int64 = 1920;
    mpv_node_list videoMap{};
    videoMap.num = 1;
    videoMap.values = &videoField;
    videoMap.keys = videoKeys;
    mpv_node videoRoot{};
    videoRoot.format = MPV_FORMAT_NODE_MAP;
    videoRoot.u.list = &videoMap;

    const QVariant videoValue = decodeNodeProperty(
        MpvPropertyId::VideoParams,
        videoRoot,
        &error);
    QVERIFY2(videoValue.isValid(), qPrintable(error));
    QCOMPARE(videoValue.toMap().value(QStringLiteral("w")).toLongLong(), qlonglong{1920});

    char audioKey[] = "samplerate";
    char* audioKeys[]{audioKey};
    mpv_node audioField{};
    audioField.format = MPV_FORMAT_INT64;
    audioField.u.int64 = 48000;
    mpv_node_list audioMap{};
    audioMap.num = 1;
    audioMap.values = &audioField;
    audioMap.keys = audioKeys;
    mpv_node audioRoot{};
    audioRoot.format = MPV_FORMAT_NODE_MAP;
    audioRoot.u.list = &audioMap;

    const QVariant audioValue = decodeNodeProperty(
        MpvPropertyId::AudioParams,
        audioRoot,
        &error);
    QVERIFY2(audioValue.isValid(), qPrintable(error));
    QCOMPARE(audioValue.toMap().value(QStringLiteral("samplerate")).toLongLong(), qlonglong{48000});
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvPropertyBaselineTest)
#include "mpv_property_baseline_test.moc"
