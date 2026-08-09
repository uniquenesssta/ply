#include "playback/infrastructure/mpv/events/mpv_media_model_mapper.h"

#include "playback/domain/events/buffering_event.h"
#include "playback/domain/events/chapter_event.h"
#include "playback/domain/events/stream_event.h"
#include "playback/domain/events/track_event.h"

#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtTest/QTest>

#include <variant>

namespace player::playback::mpv {

using namespace player::playback::domain;

class MpvMediaModelMapperTest final : public QObject
{
    Q_OBJECT

private slots:
    void mapsTrackListAndSelections();
    void mapsChaptersStreamsAndCache();
    void unavailableValuesClearMediaAxes();
    void rejectsMalformedRequiredShapes();
};

void MpvMediaModelMapperTest::mapsTrackListAndSelections()
{
    QVariantMap audio;
    audio.insert(QStringLiteral("id"), QVariant::fromValue<qlonglong>(2));
    audio.insert(QStringLiteral("type"), QStringLiteral("audio"));
    audio.insert(QStringLiteral("title"), QStringLiteral("Main Audio"));
    audio.insert(QStringLiteral("lang"), QStringLiteral("eng"));
    audio.insert(QStringLiteral("codec"), QStringLiteral("pcm_s16le"));
    audio.insert(QStringLiteral("selected"), true);
    audio.insert(QStringLiteral("default"), true);

    QVariantMap subtitle;
    subtitle.insert(QStringLiteral("id"), QVariant::fromValue<qlonglong>(4));
    subtitle.insert(QStringLiteral("type"), QStringLiteral("sub"));
    subtitle.insert(QStringLiteral("lang"), QStringLiteral("kor"));
    subtitle.insert(QStringLiteral("forced"), true);
    subtitle.insert(QStringLiteral("external"), true);
    subtitle.insert(QStringLiteral("external-filename"), QStringLiteral("sample.srt"));

    QString error;
    const auto mappedTracks = MpvMediaModelMapper::map(
        MpvPropertyChange{
            MpvPropertyId::TrackList,
            QVariant{QVariantList{audio, subtitle}}},
        &error);
    QVERIFY2(mappedTracks.has_value(), qPrintable(error));
    const auto* tracks = std::get_if<TrackListChangedEvent>(&mappedTracks->payload);
    QVERIFY(tracks != nullptr);
    QCOMPARE(tracks->tracks.size(), qsizetype{2});
    QCOMPARE(tracks->tracks.at(0).id, qint64{2});
    QCOMPARE(tracks->tracks.at(0).kind, TrackKind::Audio);
    QCOMPARE(*tracks->tracks.at(0).title, QStringLiteral("Main Audio"));
    QCOMPARE(*tracks->tracks.at(0).language, QStringLiteral("eng"));
    QVERIFY(tracks->tracks.at(0).selected);
    QVERIFY(tracks->tracks.at(0).defaultTrack);
    QCOMPARE(tracks->tracks.at(1).kind, TrackKind::Subtitle);
    QVERIFY(tracks->tracks.at(1).forced);
    QVERIFY(tracks->tracks.at(1).external);
    QCOMPARE(*tracks->tracks.at(1).externalFilename, QStringLiteral("sample.srt"));

    const auto selectedAudio = MpvMediaModelMapper::map(
        MpvPropertyChange{
            MpvPropertyId::SelectedAudioTrack,
            QVariant::fromValue<qlonglong>(2)},
        &error);
    QVERIFY2(selectedAudio.has_value(), qPrintable(error));
    const auto* audioSelection = std::get_if<SelectedAudioTrackChangedEvent>(&selectedAudio->payload);
    QVERIFY(audioSelection != nullptr);
    QVERIFY(audioSelection->trackId.has_value());
    QCOMPARE(*audioSelection->trackId, qint64{2});

    const auto disabledSubtitle = MpvMediaModelMapper::map(
        MpvPropertyChange{
            MpvPropertyId::SelectedSubtitleTrack,
            QVariant{QStringLiteral("no")}},
        &error);
    QVERIFY2(disabledSubtitle.has_value(), qPrintable(error));
    const auto* subtitleSelection = std::get_if<SelectedSubtitleTrackChangedEvent>(&disabledSubtitle->payload);
    QVERIFY(subtitleSelection != nullptr);
    QVERIFY(!subtitleSelection->trackId.has_value());
}

void MpvMediaModelMapperTest::mapsChaptersStreamsAndCache()
{
    QString error;

    QVariantMap chapter;
    chapter.insert(QStringLiteral("time"), 12.5);
    chapter.insert(QStringLiteral("title"), QStringLiteral("Chapter One"));
    const auto mappedChapters = MpvMediaModelMapper::map(
        MpvPropertyChange{
            MpvPropertyId::ChapterList,
            QVariant{QVariantList{chapter}}},
        &error);
    QVERIFY2(mappedChapters.has_value(), qPrintable(error));
    const auto* chapters = std::get_if<ChapterListChangedEvent>(&mappedChapters->payload);
    QVERIFY(chapters != nullptr);
    QCOMPARE(chapters->chapters.size(), qsizetype{1});
    QCOMPARE(chapters->chapters.front().index, qsizetype{0});
    QCOMPARE(chapters->chapters.front().startSeconds, 12.5);
    QCOMPARE(*chapters->chapters.front().title, QStringLiteral("Chapter One"));

    QVariantMap video;
    video.insert(QStringLiteral("w"), QVariant::fromValue<qlonglong>(1920));
    video.insert(QStringLiteral("h"), QVariant::fromValue<qlonglong>(1080));
    video.insert(QStringLiteral("dw"), QVariant::fromValue<qlonglong>(1920));
    video.insert(QStringLiteral("dh"), QVariant::fromValue<qlonglong>(1080));
    video.insert(QStringLiteral("aspect"), 16.0 / 9.0);
    video.insert(QStringLiteral("rotate"), QVariant::fromValue<qlonglong>(0));
    video.insert(QStringLiteral("pixelformat"), QStringLiteral("yuv420p"));
    const auto mappedVideo = MpvMediaModelMapper::map(
        MpvPropertyChange{MpvPropertyId::VideoParams, QVariant{video}},
        &error);
    QVERIFY2(mappedVideo.has_value(), qPrintable(error));
    const auto* videoEvent = std::get_if<VideoStreamInfoChangedEvent>(&mappedVideo->payload);
    QVERIFY(videoEvent != nullptr);
    QVERIFY(videoEvent->info.has_value());
    QCOMPARE(*videoEvent->info->width, qint64{1920});
    QCOMPARE(*videoEvent->info->height, qint64{1080});
    QCOMPARE(*videoEvent->info->pixelFormat, QStringLiteral("yuv420p"));

    QVariantMap audio;
    audio.insert(QStringLiteral("format"), QStringLiteral("s16"));
    audio.insert(QStringLiteral("samplerate"), QVariant::fromValue<qlonglong>(48000));
    audio.insert(QStringLiteral("channels"), QStringLiteral("stereo"));
    audio.insert(QStringLiteral("channel-count"), QVariant::fromValue<qlonglong>(2));
    audio.insert(QStringLiteral("hr-channels"), QStringLiteral("stereo"));
    const auto mappedAudio = MpvMediaModelMapper::map(
        MpvPropertyChange{MpvPropertyId::AudioParams, QVariant{audio}},
        &error);
    QVERIFY2(mappedAudio.has_value(), qPrintable(error));
    const auto* audioEvent = std::get_if<AudioStreamInfoChangedEvent>(&mappedAudio->payload);
    QVERIFY(audioEvent != nullptr);
    QVERIFY(audioEvent->info.has_value());
    QCOMPARE(*audioEvent->info->sampleRate, qint64{48000});
    QCOMPARE(*audioEvent->info->channelCount, qint64{2});

    QVariantMap cache;
    cache.insert(QStringLiteral("bof-cached"), true);
    cache.insert(QStringLiteral("eof-cached"), false);
    cache.insert(QStringLiteral("fw-bytes"), QVariant::fromValue<qlonglong>(4096));
    cache.insert(QStringLiteral("total-bytes"), QVariant::fromValue<qlonglong>(8192));
    cache.insert(QStringLiteral("cache-end"), 20.0);
    cache.insert(QStringLiteral("reader-pts"), 15.5);
    cache.insert(QStringLiteral("cache-duration"), 4.5);
    cache.insert(QStringLiteral("raw-input-rate"), QVariant::fromValue<qlonglong>(128000));
    const auto mappedCache = MpvMediaModelMapper::map(
        MpvPropertyChange{MpvPropertyId::DemuxerCacheState, QVariant{cache}},
        &error);
    QVERIFY2(mappedCache.has_value(), qPrintable(error));
    const auto* cacheEvent = std::get_if<CacheStatusChangedEvent>(&mappedCache->payload);
    QVERIFY(cacheEvent != nullptr);
    QVERIFY(cacheEvent->status.has_value());
    QVERIFY(*cacheEvent->status->beginningCached);
    QVERIFY(!*cacheEvent->status->endCached);
    QCOMPARE(*cacheEvent->status->forwardBytes, qint64{4096});
    QCOMPARE(*cacheEvent->status->durationSeconds, 4.5);
}

void MpvMediaModelMapperTest::unavailableValuesClearMediaAxes()
{
    QString error;

    const auto tracks = MpvMediaModelMapper::map(
        MpvPropertyChange{MpvPropertyId::TrackList, std::monostate{}},
        &error);
    QVERIFY2(tracks.has_value(), qPrintable(error));
    const auto* trackEvent = std::get_if<TrackListChangedEvent>(&tracks->payload);
    QVERIFY(trackEvent != nullptr);
    QVERIFY(trackEvent->tracks.isEmpty());

    const auto video = MpvMediaModelMapper::map(
        MpvPropertyChange{MpvPropertyId::VideoParams, std::monostate{}},
        &error);
    QVERIFY2(video.has_value(), qPrintable(error));
    const auto* videoEvent = std::get_if<VideoStreamInfoChangedEvent>(&video->payload);
    QVERIFY(videoEvent != nullptr);
    QVERIFY(!videoEvent->info.has_value());

    const auto cache = MpvMediaModelMapper::map(
        MpvPropertyChange{MpvPropertyId::DemuxerCacheState, QVariant{}},
        &error);
    QVERIFY2(cache.has_value(), qPrintable(error));
    const auto* cacheEvent = std::get_if<CacheStatusChangedEvent>(&cache->payload);
    QVERIFY(cacheEvent != nullptr);
    QVERIFY(!cacheEvent->status.has_value());
}

void MpvMediaModelMapperTest::rejectsMalformedRequiredShapes()
{
    QString error;

    QVariantMap missingTrackId;
    missingTrackId.insert(QStringLiteral("type"), QStringLiteral("audio"));
    const auto track = MpvMediaModelMapper::map(
        MpvPropertyChange{
            MpvPropertyId::TrackList,
            QVariant{QVariantList{missingTrackId}}},
        &error);
    QVERIFY(!track.has_value());
    QVERIFY(!error.isEmpty());

    QVariantMap malformedChapter;
    malformedChapter.insert(QStringLiteral("time"), QStringLiteral("not-a-number"));
    const auto chapter = MpvMediaModelMapper::map(
        MpvPropertyChange{
            MpvPropertyId::ChapterList,
            QVariant{QVariantList{malformedChapter}}},
        &error);
    QVERIFY(!chapter.has_value());
    QVERIFY(!error.isEmpty());
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvMediaModelMapperTest)
#include "mpv_media_model_mapper_test.moc"
