#include "playback/domain/events/chapter_event.h"
#include "playback/infrastructure/mpv/events/media_model_mapping/mpv_chapter_list_decoder.h"
#include "playback/infrastructure/mpv/events/media_model_mapping/mpv_chapter_model_mapper.h"

#include <QtTest>

#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <variant>

namespace player::playback::mpv {
namespace {

QVariantMap chapter(double time, const QString& title = {})
{
    QVariantMap result{
        {QStringLiteral("time"), time},
    };
    if (!title.isNull()) {
        result.insert(QStringLiteral("title"), title);
    }
    return result;
}

} // namespace

class MpvChapterListDecoderTest final : public QObject
{
    Q_OBJECT

private slots:
    void emptyListDecodesAsStableEmpty();
    void preservesOrderDuplicateTimesLongTitlesAndMissingTitle();
    void ignoresUnknownBackendFields();
    void malformedPayloadsAreRejectedAtomically();
    void mapperKeepsUnavailableAndValidListSemantics();
};

void MpvChapterListDecoderTest::emptyListDecodesAsStableEmpty()
{
    QString error;
    const auto decoded = MpvChapterListDecoder::decode(QVariant{QVariantList{}}, &error);

    QVERIFY2(decoded.has_value(), qPrintable(error));
    QVERIFY(decoded->isEmpty());
}

void MpvChapterListDecoderTest::preservesOrderDuplicateTimesLongTitlesAndMissingTitle()
{
    const QString longTitle = QStringLiteral(
        "A deliberately long chapter title that must pass through the decoder without truncation, "
        "normalization, reordering, or presentation-layer rewriting.");

    QVariantMap untitled{
        {QStringLiteral("time"), 42.5},
    };

    QString error;
    const auto decoded = MpvChapterListDecoder::decode(
        QVariant{QVariantList{
            chapter(0.0, QStringLiteral("Opening")),
            chapter(42.5, longTitle),
            untitled,
            chapter(90.0, QStringLiteral("Finale")),
        }},
        &error);

    QVERIFY2(decoded.has_value(), qPrintable(error));
    QCOMPARE(decoded->size(), 4);

    QCOMPARE(decoded->at(0).index, qsizetype{0});
    QCOMPARE(decoded->at(0).startSeconds, 0.0);
    QVERIFY(decoded->at(0).title.has_value());
    QCOMPARE(*decoded->at(0).title, QStringLiteral("Opening"));

    QCOMPARE(decoded->at(1).index, qsizetype{1});
    QCOMPARE(decoded->at(1).startSeconds, 42.5);
    QVERIFY(decoded->at(1).title.has_value());
    QCOMPARE(*decoded->at(1).title, longTitle);

    QCOMPARE(decoded->at(2).index, qsizetype{2});
    QCOMPARE(decoded->at(2).startSeconds, 42.5);
    QVERIFY(!decoded->at(2).title.has_value());

    QCOMPARE(decoded->at(3).index, qsizetype{3});
    QCOMPARE(decoded->at(3).startSeconds, 90.0);
}

void MpvChapterListDecoderTest::ignoresUnknownBackendFields()
{
    QVariantMap item = chapter(12.0, QStringLiteral("Known"));
    item.insert(QStringLiteral("future-field"), QStringLiteral("future-value"));

    QString error;
    const auto decoded = MpvChapterListDecoder::decode(
        QVariant{QVariantList{item}},
        &error);

    QVERIFY2(decoded.has_value(), qPrintable(error));
    QCOMPARE(decoded->size(), 1);
    QCOMPARE(decoded->front().startSeconds, 12.0);
}

void MpvChapterListDecoderTest::malformedPayloadsAreRejectedAtomically()
{
    QVariantMap missingTime{
        {QStringLiteral("title"), QStringLiteral("Missing Time")},
    };
    QVariantMap invalidTime{
        {QStringLiteral("time"), QStringLiteral("soon")},
    };
    QVariantMap negativeTime{
        {QStringLiteral("time"), -1.0},
    };
    QVariantMap invalidTitle{
        {QStringLiteral("time"), 1.0},
        {QStringLiteral("title"), 42},
    };

    const QList<QVariant> malformed{
        QVariant{QStringLiteral("not-an-array")},
        QVariant{QVariantList{QVariant{QStringLiteral("not-a-map")}}},
        QVariant{QVariantList{missingTime}},
        QVariant{QVariantList{invalidTime}},
        QVariant{QVariantList{negativeTime}},
        QVariant{QVariantList{invalidTitle}},
    };

    for (const QVariant& payload : malformed) {
        QString error;
        const auto decoded = MpvChapterListDecoder::decode(payload, &error);
        QVERIFY(!decoded.has_value());
        QVERIFY2(!error.isEmpty(), "Malformed chapter-list must return a diagnostic.");
    }
}

void MpvChapterListDecoderTest::mapperKeepsUnavailableAndValidListSemantics()
{
    using player::playback::domain::ChapterListChangedEvent;

    const auto unavailable = MpvChapterModelMapper::map(
        MpvPropertyValue{std::monostate{}},
        nullptr);
    QVERIFY(unavailable.has_value());
    const auto* unavailableEvent = std::get_if<ChapterListChangedEvent>(&unavailable->payload);
    QVERIFY(unavailableEvent != nullptr);
    QVERIFY(unavailableEvent->chapters.isEmpty());

    QString error;
    const auto mapped = MpvChapterModelMapper::map(
        MpvPropertyValue{QVariant{QVariantList{
            chapter(0.0, QStringLiteral("Opening")),
            chapter(15.0, QStringLiteral("Part Two")),
        }}},
        &error);
    QVERIFY2(mapped.has_value(), qPrintable(error));

    const auto* event = std::get_if<ChapterListChangedEvent>(&mapped->payload);
    QVERIFY(event != nullptr);
    QCOMPARE(event->chapters.size(), 2);
    QCOMPARE(event->chapters.at(0).index, qsizetype{0});
    QCOMPARE(event->chapters.at(1).index, qsizetype{1});
    QCOMPARE(event->chapters.at(1).startSeconds, 15.0);
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvChapterListDecoderTest)
#include "mpv_chapter_list_decoder_test.moc"
