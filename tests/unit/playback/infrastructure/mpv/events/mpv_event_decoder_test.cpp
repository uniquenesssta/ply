#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/commands/mpv_command_executor.h"
#include "playback/infrastructure/mpv/commands/mpv_command_request.h"
#include "playback/infrastructure/mpv/errors/mpv_error_mapper.h"
#include "playback/infrastructure/mpv/events/mpv_event.h"
#include "playback/infrastructure/mpv/events/mpv_event_decoder.h"
#include "playback/infrastructure/mpv/events/mpv_event_loop.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/initialization/mpv_option_profile.h"
#include "playback/infrastructure/mpv/properties/mpv_property_observer.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <mpv/client.h>

#include <QFile>
#include <QList>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QtTest>

#include <memory>

namespace player::playback::mpv {
namespace {

void appendLittleEndian16(QByteArray& bytes, quint16 value)
{
    bytes.append(static_cast<char>(value & 0xffU));
    bytes.append(static_cast<char>((value >> 8U) & 0xffU));
}

void appendLittleEndian32(QByteArray& bytes, quint32 value)
{
    bytes.append(static_cast<char>(value & 0xffU));
    bytes.append(static_cast<char>((value >> 8U) & 0xffU));
    bytes.append(static_cast<char>((value >> 16U) & 0xffU));
    bytes.append(static_cast<char>((value >> 24U) & 0xffU));
}

QByteArray makeSilentPcmWav()
{
    constexpr quint16 kChannels = 1;
    constexpr quint32 kSampleRate = 8000;
    constexpr quint16 kBitsPerSample = 16;
    constexpr quint32 kSampleCount = 1600;
    constexpr quint16 kBlockAlign = kChannels * (kBitsPerSample / 8U);
    constexpr quint32 kByteRate = kSampleRate * kBlockAlign;
    constexpr quint32 kDataSize = kSampleCount * kBlockAlign;

    QByteArray bytes;
    bytes.reserve(static_cast<qsizetype>(44U + kDataSize));
    bytes.append("RIFF", 4);
    appendLittleEndian32(bytes, 36U + kDataSize);
    bytes.append("WAVE", 4);
    bytes.append("fmt ", 4);
    appendLittleEndian32(bytes, 16U);
    appendLittleEndian16(bytes, 1U);
    appendLittleEndian16(bytes, kChannels);
    appendLittleEndian32(bytes, kSampleRate);
    appendLittleEndian32(bytes, kByteRate);
    appendLittleEndian16(bytes, kBlockAlign);
    appendLittleEndian16(bytes, kBitsPerSample);
    bytes.append(QByteArray(static_cast<qsizetype>(kDataSize), '\0'));
    return bytes;
}

std::unique_ptr<MpvHandle> createHeadlessInitializedHandle(QString* errorMessage)
{
    auto handle = MpvHandle::create(errorMessage);
    if (handle == nullptr) {
        return {};
    }

    const MpvOptionProfile profile(QList<MpvOption>{
        {QByteArrayLiteral("config"), QByteArrayLiteral("no")},
        {QByteArrayLiteral("ao"), QByteArrayLiteral("null")},
        {QByteArrayLiteral("vo"), QByteArrayLiteral("null")},
    });
    if (!MpvInitializer::initialize(*handle, profile, errorMessage)) {
        return {};
    }
    return handle;
}

bool containsEventType(const QSignalSpy& spy, MpvEventType type)
{
    for (const QList<QVariant>& arguments : spy) {
        if (arguments.isEmpty()) {
            continue;
        }
        if (qvariant_cast<MpvEvent>(arguments.first()).type == type) {
            return true;
        }
    }
    return false;
}

bool containsCommandReply(const QSignalSpy& spy, quint64 requestId)
{
    for (const QList<QVariant>& arguments : spy) {
        if (arguments.isEmpty()) {
            continue;
        }
        const MpvEvent event = qvariant_cast<MpvEvent>(arguments.first());
        if (event.type == MpvEventType::CommandReply && event.replyUserdata == requestId) {
            return true;
        }
    }
    return false;
}

bool containsEndReason(const QSignalSpy& spy, MpvEndFileReason reason)
{
    for (const QList<QVariant>& arguments : spy) {
        if (arguments.isEmpty()) {
            continue;
        }
        const MpvEvent event = qvariant_cast<MpvEvent>(arguments.first());
        if (event.type != MpvEventType::EndFile) {
            continue;
        }
        const auto* endFile = std::get_if<MpvEndFileData>(&event.payload);
        if (endFile != nullptr && endFile->reason == reason) {
            return true;
        }
    }
    return false;
}

} // namespace

class MpvEventDecoderTest final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void errorMapperPreservesKnownAndUnknownCodes();
    void decodesSyntheticCoreEvents();
    void decodesPropertyNodePayloadWithoutRawPointers();
    void unknownAndMalformedEventsAreSafe();
    void realShortMediaSequenceUsesTypedEvents();
};

void MpvEventDecoderTest::initTestCase()
{
    qRegisterMetaType<MpvEvent>();
}

void MpvEventDecoderTest::errorMapperPreservesKnownAndUnknownCodes()
{
    const MpvError success = MpvErrorMapper::map(MPV_ERROR_SUCCESS);
    QCOMPARE(success.code, MpvErrorCode::Success);
    QVERIFY(success.isSuccess());

    const MpvError loading = MpvErrorMapper::map(MPV_ERROR_LOADING_FAILED);
    QCOMPARE(loading.code, MpvErrorCode::LoadingFailed);
    QCOMPARE(loading.rawCode, MPV_ERROR_LOADING_FAILED);
    QVERIFY(!loading.message.isEmpty());
    QVERIFY(!loading.isSuccess());

    const MpvError unknown = MpvErrorMapper::map(-9999);
    QCOMPARE(unknown.code, MpvErrorCode::Unknown);
    QCOMPARE(unknown.rawCode, -9999);
    QVERIFY(!unknown.message.isEmpty());
}

void MpvEventDecoderTest::decodesSyntheticCoreEvents()
{
    mpv_event_start_file startData{};
    startData.playlist_entry_id = 42;
    mpv_event startEvent{};
    startEvent.event_id = MPV_EVENT_START_FILE;
    startEvent.data = &startData;
    MpvEvent decoded = MpvEventDecoder::decode(startEvent);
    QCOMPARE(decoded.type, MpvEventType::StartFile);
    QCOMPARE(std::get<MpvStartFileData>(decoded.payload).playlistEntryId, qint64{42});

    mpv_event fileLoaded{};
    fileLoaded.event_id = MPV_EVENT_FILE_LOADED;
    QCOMPARE(MpvEventDecoder::decode(fileLoaded).type, MpvEventType::FileLoaded);

    mpv_event commandReply{};
    commandReply.event_id = MPV_EVENT_COMMAND_REPLY;
    commandReply.error = MPV_ERROR_COMMAND;
    commandReply.reply_userdata = 77;
    decoded = MpvEventDecoder::decode(commandReply);
    QCOMPARE(decoded.type, MpvEventType::CommandReply);
    QCOMPARE(decoded.replyUserdata, quint64{77});
    QCOMPARE(decoded.error.code, MpvErrorCode::Command);

    mpv_event_end_file endData{};
    endData.reason = MPV_END_FILE_REASON_ERROR;
    endData.error = MPV_ERROR_LOADING_FAILED;
    endData.playlist_entry_id = 42;
    endData.playlist_insert_id = 100;
    endData.playlist_insert_num_entries = 3;
    mpv_event endEvent{};
    endEvent.event_id = MPV_EVENT_END_FILE;
    endEvent.data = &endData;
    decoded = MpvEventDecoder::decode(endEvent);
    QCOMPARE(decoded.type, MpvEventType::EndFile);
    QCOMPARE(decoded.error.code, MpvErrorCode::LoadingFailed);
    const MpvEndFileData endFile = std::get<MpvEndFileData>(decoded.payload);
    QCOMPARE(endFile.reason, MpvEndFileReason::Error);
    QCOMPARE(endFile.playlistEntryId, qint64{42});
    QCOMPARE(endFile.playlistInsertId, qint64{100});
    QCOMPARE(endFile.playlistInsertNumEntries, 3);

    mpv_event_log_message logData{};
    logData.prefix = "core";
    logData.level = "info";
    logData.text = "typed log message\n";
    logData.log_level = MPV_LOG_LEVEL_INFO;
    mpv_event logEvent{};
    logEvent.event_id = MPV_EVENT_LOG_MESSAGE;
    logEvent.data = &logData;
    decoded = MpvEventDecoder::decode(logEvent);
    QCOMPARE(decoded.type, MpvEventType::LogMessage);
    const MpvLogMessageData logMessage = std::get<MpvLogMessageData>(decoded.payload);
    QCOMPARE(logMessage.prefix, QStringLiteral("core"));
    QCOMPARE(logMessage.level, QStringLiteral("info"));
    QCOMPARE(logMessage.text, QStringLiteral("typed log message\n"));
    QCOMPARE(logMessage.numericLevel, static_cast<int>(MPV_LOG_LEVEL_INFO));

    mpv_event shutdown{};
    shutdown.event_id = MPV_EVENT_SHUTDOWN;
    QCOMPARE(MpvEventDecoder::decode(shutdown).type, MpvEventType::Shutdown);
}

void MpvEventDecoderTest::decodesPropertyNodePayloadWithoutRawPointers()
{
    const MpvPropertyDefinition* trackList = MpvPropertyRegistry::findById(MpvPropertyId::TrackList);
    QVERIFY(trackList != nullptr);

    char idKey[] = "id";
    char titleKey[] = "title";
    char titleValue[] = "Synthetic Track";
    char* keys[]{idKey, titleKey};

    mpv_node fields[2]{};
    fields[0].format = MPV_FORMAT_INT64;
    fields[0].u.int64 = 7;
    fields[1].format = MPV_FORMAT_STRING;
    fields[1].u.string = titleValue;

    mpv_node_list trackMap{};
    trackMap.num = 2;
    trackMap.values = fields;
    trackMap.keys = keys;

    mpv_node track{};
    track.format = MPV_FORMAT_NODE_MAP;
    track.u.list = &trackMap;

    mpv_node_list trackArray{};
    trackArray.num = 1;
    trackArray.values = &track;

    mpv_node root{};
    root.format = MPV_FORMAT_NODE_ARRAY;
    root.u.list = &trackArray;

    mpv_event_property property{};
    property.name = trackList->name.constData();
    property.format = MPV_FORMAT_NODE;
    property.data = &root;

    mpv_event rawEvent{};
    rawEvent.event_id = MPV_EVENT_PROPERTY_CHANGE;
    rawEvent.reply_userdata = trackList->observationId;
    rawEvent.data = &property;

    const MpvEvent decoded = MpvEventDecoder::decode(rawEvent);
    QCOMPARE(decoded.type, MpvEventType::PropertyChange);
    const MpvPropertyChange change = std::get<MpvPropertyChange>(decoded.payload);
    QCOMPARE(change.id, MpvPropertyId::TrackList);
    const QVariant* nodeValue = std::get_if<QVariant>(&change.value);
    QVERIFY(nodeValue != nullptr);

    const QVariantList tracks = nodeValue->toList();
    QCOMPARE(tracks.size(), qsizetype{1});
    const QVariantMap firstTrack = tracks.first().toMap();
    QCOMPARE(firstTrack.value(QStringLiteral("id")).toLongLong(), qlonglong{7});
    QCOMPARE(firstTrack.value(QStringLiteral("title")).toString(), QStringLiteral("Synthetic Track"));
}

void MpvEventDecoderTest::unknownAndMalformedEventsAreSafe()
{
    mpv_event unknown{};
    unknown.event_id = static_cast<mpv_event_id>(9999);
    const MpvEvent unknownDecoded = MpvEventDecoder::decode(unknown);
    QCOMPARE(unknownDecoded.type, MpvEventType::Unknown);
    QCOMPARE(std::get<MpvUnknownEventData>(unknownDecoded.payload).rawEventId, 9999);

    mpv_event malformedLog{};
    malformedLog.event_id = MPV_EVENT_LOG_MESSAGE;
    const MpvEvent malformedDecoded = MpvEventDecoder::decode(malformedLog);
    QCOMPARE(malformedDecoded.type, MpvEventType::DecodeFailure);
    const MpvDecodeFailureData failure = std::get<MpvDecodeFailureData>(malformedDecoded.payload);
    QCOMPARE(failure.rawEventId, static_cast<int>(MPV_EVENT_LOG_MESSAGE));
    QVERIFY(!failure.diagnostic.isEmpty());
}

void MpvEventDecoderTest::realShortMediaSequenceUsesTypedEvents()
{
    QString error;
    auto handle = createHeadlessInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString mediaPath = temporaryDirectory.filePath(QStringLiteral("r2-event-sequence.wav"));
    QFile media(mediaPath);
    QVERIFY(media.open(QIODevice::WriteOnly));
    const QByteArray wavBytes = makeSilentPcmWav();
    QCOMPARE(media.write(wavBytes), static_cast<qint64>(wavBytes.size()));
    media.close();

    MpvEventLoop eventLoop(*handle);
    QSignalSpy eventSpy(&eventLoop, &MpvEventLoop::eventDecoded);
    QVERIFY2(eventLoop.start(&error), qPrintable(error));

    MpvPropertyObserver observer(*handle);
    QVERIFY2(observer.start(&error), qPrintable(error));

    MpvCommandExecutor executor(*handle);
    constexpr quint64 kLoadRequestId = 7001;
    QVERIFY2(executor.submit(kLoadRequestId, MpvLoadRequest{mediaPath}, &error), qPrintable(error));

    QTRY_VERIFY_WITH_TIMEOUT(containsCommandReply(eventSpy, kLoadRequestId), 5000);
    QTRY_VERIFY_WITH_TIMEOUT(containsEventType(eventSpy, MpvEventType::StartFile), 5000);
    QTRY_VERIFY_WITH_TIMEOUT(containsEventType(eventSpy, MpvEventType::FileLoaded), 5000);
    QTRY_VERIFY_WITH_TIMEOUT(containsEventType(eventSpy, MpvEventType::PropertyChange), 5000);
    QTRY_VERIFY_WITH_TIMEOUT(containsEventType(eventSpy, MpvEventType::EndFile), 5000);
    QVERIFY(containsEndReason(eventSpy, MpvEndFileReason::Eof));
    QVERIFY(!containsEventType(eventSpy, MpvEventType::DecodeFailure));

    QVERIFY2(observer.stop(&error), qPrintable(error));
    eventLoop.stop();
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvEventDecoderTest)
#include "mpv_event_decoder_test.moc"
