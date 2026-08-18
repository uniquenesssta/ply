#include "playback/domain/events/track_event.h"
#include "playback/infrastructure/mpv/events/media_model_mapping/mpv_track_list_decoder.h"
#include "playback/infrastructure/mpv/events/media_model_mapping/mpv_track_model_mapper.h"

#include <QtTest>

#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <variant>

namespace player::playback::mpv {
namespace {

QVariantMap baseTrack(qlonglong id, const QString& type)
{
    return QVariantMap{
        {QStringLiteral("id"), QVariant::fromValue(id)},
        {QStringLiteral("type"), type},
    };
}

int kindValue(player::playback::domain::TrackKind kind)
{
    return static_cast<int>(kind);
}

} // namespace

class MpvTrackListDecoderTest final : public QObject
{
    Q_OBJECT

private slots:
    void decodesMultipleTracksAndOptionalMetadata();
    void missingOptionalFieldsUseProductDefaults();
    void ignoresUnknownBackendFields();
    void emptyListDecodesAsEmpty();
    void malformedPayloadsAreRejectedAtomically();
    void mapperKeepsUnavailableAndValidListSemantics();
};

void MpvTrackListDecoderTest::decodesMultipleTracksAndOptionalMetadata()
{
    QVariantMap audio = baseTrack(2, QStringLiteral("audio"));
    audio.insert(QStringLiteral("title"), QStringLiteral("Director Commentary"));
    audio.insert(QStringLiteral("lang"), QStringLiteral("eng"));
    audio.insert(QStringLiteral("codec"), QStringLiteral("aac"));
    audio.insert(QStringLiteral("selected"), true);
    audio.insert(QStringLiteral("default"), true);

    QVariantMap subtitle = baseTrack(5, QStringLiteral("sub"));
    subtitle.insert(QStringLiteral("title"), QStringLiteral("Signs & Songs"));
    subtitle.insert(QStringLiteral("lang"), QStringLiteral("jpn"));
    subtitle.insert(QStringLiteral("codec"), QStringLiteral("ass"));
    subtitle.insert(QStringLiteral("external-filename"), QStringLiteral("C:/media/subs/signs.ass"));
    subtitle.insert(QStringLiteral("forced"), true);
    subtitle.insert(QStringLiteral("external"), true);

    QString error;
    const auto decoded = MpvTrackListDecoder::decode(
        QVariant{QVariantList{audio, subtitle}},
        &error);

    QVERIFY2(decoded.has_value(), qPrintable(error));
    QCOMPARE(decoded->size(), 2);

    const auto& first = decoded->at(0);
    QCOMPARE(first.id, qint64{2});
    QCOMPARE(kindValue(first.kind), kindValue(player::playback::domain::TrackKind::Audio));
    QVERIFY(first.title.has_value());
    QCOMPARE(*first.title, QStringLiteral("Director Commentary"));
    QVERIFY(first.language.has_value());
    QCOMPARE(*first.language, QStringLiteral("eng"));
    QVERIFY(first.codec.has_value());
    QCOMPARE(*first.codec, QStringLiteral("aac"));
    QVERIFY(first.selected);
    QVERIFY(first.defaultTrack);
    QVERIFY(!first.forced);
    QVERIFY(!first.external);

    const auto& second = decoded->at(1);
    QCOMPARE(second.id, qint64{5});
    QCOMPARE(kindValue(second.kind), kindValue(player::playback::domain::TrackKind::Subtitle));
    QVERIFY(second.externalFilename.has_value());
    QCOMPARE(*second.externalFilename, QStringLiteral("C:/media/subs/signs.ass"));
    QVERIFY(second.forced);
    QVERIFY(second.external);
    QVERIFY(!second.selected);
}

void MpvTrackListDecoderTest::missingOptionalFieldsUseProductDefaults()
{
    const QVariantMap video = baseTrack(1, QStringLiteral("video"));

    QString error;
    const auto decoded = MpvTrackListDecoder::decode(
        QVariant{QVariantList{video}},
        &error);

    QVERIFY2(decoded.has_value(), qPrintable(error));
    QCOMPARE(decoded->size(), 1);
    const auto& track = decoded->front();
    QCOMPARE(track.id, qint64{1});
    QCOMPARE(kindValue(track.kind), kindValue(player::playback::domain::TrackKind::Video));
    QVERIFY(!track.title.has_value());
    QVERIFY(!track.language.has_value());
    QVERIFY(!track.codec.has_value());
    QVERIFY(!track.externalFilename.has_value());
    QVERIFY(!track.selected);
    QVERIFY(!track.defaultTrack);
    QVERIFY(!track.forced);
    QVERIFY(!track.external);
    QVERIFY(!track.image);
    QVERIFY(!track.albumArt);
}

void MpvTrackListDecoderTest::ignoresUnknownBackendFields()
{
    QVariantMap audio = baseTrack(3, QStringLiteral("audio"));
    audio.insert(QStringLiteral("demux-channel-count"), 6);
    audio.insert(QStringLiteral("future-field"), QStringLiteral("future-value"));

    QString error;
    const auto decoded = MpvTrackListDecoder::decode(
        QVariant{QVariantList{audio}},
        &error);

    QVERIFY2(decoded.has_value(), qPrintable(error));
    QCOMPARE(decoded->size(), 1);
    QCOMPARE(decoded->front().id, qint64{3});
}

void MpvTrackListDecoderTest::emptyListDecodesAsEmpty()
{
    QString error;
    const auto decoded = MpvTrackListDecoder::decode(QVariant{QVariantList{}}, &error);
    QVERIFY2(decoded.has_value(), qPrintable(error));
    QVERIFY(decoded->isEmpty());
}

void MpvTrackListDecoderTest::malformedPayloadsAreRejectedAtomically()
{
    QVariantMap missingId;
    missingId.insert(QStringLiteral("type"), QStringLiteral("audio"));

    QVariantMap zeroId = baseTrack(0, QStringLiteral("audio"));

    QVariantMap missingType;
    missingType.insert(QStringLiteral("id"), QVariant::fromValue<qlonglong>(1));

    QVariantMap unsupportedType = baseTrack(1, QStringLiteral("data"));

    QVariantMap invalidTitle = baseTrack(1, QStringLiteral("audio"));
    invalidTitle.insert(QStringLiteral("title"), 42);

    QVariantMap invalidSelected = baseTrack(1, QStringLiteral("sub"));
    invalidSelected.insert(QStringLiteral("selected"), QStringLiteral("yes"));

    const QList<QVariant> malformed{
        QVariant{QStringLiteral("not-an-array")},
        QVariant{QVariantList{QVariant{QStringLiteral("not-a-map")}}},
        QVariant{QVariantList{missingId}},
        QVariant{QVariantList{zeroId}},
        QVariant{QVariantList{missingType}},
        QVariant{QVariantList{unsupportedType}},
        QVariant{QVariantList{invalidTitle}},
        QVariant{QVariantList{invalidSelected}},
    };

    for (const QVariant& payload : malformed) {
        QString error;
        const auto decoded = MpvTrackListDecoder::decode(payload, &error);
        QVERIFY(!decoded.has_value());
        QVERIFY2(!error.isEmpty(), "Malformed track-list must return a diagnostic.");
    }
}

void MpvTrackListDecoderTest::mapperKeepsUnavailableAndValidListSemantics()
{
    using player::playback::domain::TrackListChangedEvent;

    const auto unavailable = MpvTrackModelMapper::mapTrackList(
        MpvPropertyValue{std::monostate{}},
        nullptr);
    QVERIFY(unavailable.has_value());
    const auto* unavailableEvent = std::get_if<TrackListChangedEvent>(&unavailable->payload);
    QVERIFY(unavailableEvent != nullptr);
    QVERIFY(unavailableEvent->tracks.isEmpty());

    QVariantMap subtitle = baseTrack(8, QStringLiteral("sub"));
    subtitle.insert(QStringLiteral("lang"), QStringLiteral("chi"));

    QString error;
    const auto mapped = MpvTrackModelMapper::mapTrackList(
        MpvPropertyValue{QVariant{QVariantList{subtitle}}},
        &error);
    QVERIFY2(mapped.has_value(), qPrintable(error));

    const auto* event = std::get_if<TrackListChangedEvent>(&mapped->payload);
    QVERIFY(event != nullptr);
    QCOMPARE(event->tracks.size(), 1);
    QCOMPARE(event->tracks.front().id, qint64{8});
    QCOMPARE(kindValue(event->tracks.front().kind), kindValue(player::playback::domain::TrackKind::Subtitle));
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvTrackListDecoderTest)
#include "mpv_track_list_decoder_test.moc"
