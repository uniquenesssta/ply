#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/commands/mpv_command_encoder.h"
#include "playback/infrastructure/mpv/commands/mpv_command_executor.h"
#include "playback/infrastructure/mpv/commands/mpv_command_request.h"
#include "playback/infrastructure/mpv/events/mpv_event.h"
#include "playback/infrastructure/mpv/events/mpv_event_loop.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"

#include <QSet>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QtTest>

#include <limits>
#include <memory>
#include <optional>
#include <thread>

namespace player::playback::mpv {
namespace {

std::unique_ptr<MpvHandle> createInitializedHandle(QString* errorMessage)
{
    auto handle = MpvHandle::create(errorMessage);
    if (handle == nullptr) {
        return {};
    }

    if (!MpvInitializer::initializeProduct(*handle, errorMessage)) {
        return {};
    }

    return handle;
}

bool containsCommandReplies(
    const QSignalSpy& spy,
    const QSet<quint64>& expectedRequestIds)
{
    QSet<quint64> observedRequestIds;
    for (const QList<QVariant>& arguments : spy) {
        if (arguments.isEmpty()) {
            continue;
        }
        const MpvEvent event = qvariant_cast<MpvEvent>(arguments.first());
        if (event.type == MpvEventType::CommandReply) {
            observedRequestIds.insert(event.replyUserdata);
        }
    }

    for (quint64 requestId : expectedRequestIds) {
        if (!observedRequestIds.contains(requestId)) {
            return false;
        }
    }
    return true;
}

} // namespace

class MpvCommandExecutorTest final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void encoderProducesExpectedCommands();
    void encoderRejectsInvalidRequests();
    void executorRequiresInitializedHandle();
    void executorRejectsSubmissionFromAnotherThread();
    void asyncSubmissionRepliesForMvpCommands();
};

void MpvCommandExecutorTest::initTestCase()
{
    qRegisterMetaType<MpvEvent>();
}

void MpvCommandExecutorTest::encoderProducesExpectedCommands()
{
    QString error;

    auto encoded = MpvCommandEncoder::encode(MpvLoadRequest{QStringLiteral("sample.mp4")}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("loadfile"), QByteArrayLiteral("sample.mp4"), QByteArrayLiteral("replace")}));

    encoded = MpvCommandEncoder::encode(MpvPlayRequest{}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("set"), QByteArrayLiteral("pause"), QByteArrayLiteral("no")}));

    encoded = MpvCommandEncoder::encode(MpvPauseRequest{}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("set"), QByteArrayLiteral("pause"), QByteArrayLiteral("yes")}));

    encoded = MpvCommandEncoder::encode(MpvStopRequest{}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(*encoded, (QList<QByteArray>{QByteArrayLiteral("stop")}));

    encoded = MpvCommandEncoder::encode(MpvSeekRequest{12.5, MpvSeekMode::Absolute}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("seek"), QByteArrayLiteral("12.5"), QByteArrayLiteral("absolute+exact")}));

    encoded = MpvCommandEncoder::encode(MpvSeekRequest{-2.5, MpvSeekMode::Relative}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("seek"), QByteArrayLiteral("-2.5"), QByteArrayLiteral("relative+exact")}));

    encoded = MpvCommandEncoder::encode(MpvVolumeRequest{75.0}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("set"), QByteArrayLiteral("volume"), QByteArrayLiteral("75")}));

    encoded = MpvCommandEncoder::encode(MpvMuteRequest{true}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("set"), QByteArrayLiteral("mute"), QByteArrayLiteral("yes")}));

    encoded = MpvCommandEncoder::encode(MpvSpeedRequest{1.25}, &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("set"), QByteArrayLiteral("speed"), QByteArrayLiteral("1.25")}));

    encoded = MpvCommandEncoder::encode(
        MpvExternalSubtitleRequest{QStringLiteral("C:/media/captions.ass")},
        &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("sub-add"), QByteArrayLiteral("C:/media/captions.ass"), QByteArrayLiteral("cached")}));

    encoded = MpvCommandEncoder::encode(
        MpvTrackSelectionRequest{MpvTrackSelectionKind::Audio, qint64{7}},
        &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("set"), QByteArrayLiteral("aid"), QByteArrayLiteral("7")}));

    encoded = MpvCommandEncoder::encode(
        MpvTrackSelectionRequest{MpvTrackSelectionKind::Subtitle, qint64{11}},
        &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("set"), QByteArrayLiteral("sid"), QByteArrayLiteral("11")}));

    encoded = MpvCommandEncoder::encode(
        MpvTrackSelectionRequest{MpvTrackSelectionKind::Subtitle, std::nullopt},
        &error);
    QVERIFY2(encoded.has_value(), qPrintable(error));
    QCOMPARE(
        *encoded,
        (QList<QByteArray>{QByteArrayLiteral("set"), QByteArrayLiteral("sid"), QByteArrayLiteral("no")}));
}

void MpvCommandExecutorTest::encoderRejectsInvalidRequests()
{
    QString error;

    QVERIFY(!MpvCommandEncoder::encode(MpvLoadRequest{}, &error).has_value());
    QVERIFY(!error.isEmpty());

    QString embeddedNull = QStringLiteral("media");
    embeddedNull.append(QChar(u'\0'));
    embeddedNull.append(QStringLiteral(".mp4"));
    QVERIFY(!MpvCommandEncoder::encode(MpvLoadRequest{embeddedNull}, &error).has_value());
    QVERIFY(!error.isEmpty());

    QVERIFY(!MpvCommandEncoder::encode(MpvExternalSubtitleRequest{}, &error).has_value());
    QVERIFY(!error.isEmpty());

    QString subtitleWithNull = QStringLiteral("captions");
    subtitleWithNull.append(QChar(u'\0'));
    subtitleWithNull.append(QStringLiteral(".srt"));
    QVERIFY(!MpvCommandEncoder::encode(
        MpvExternalSubtitleRequest{subtitleWithNull},
        &error).has_value());
    QVERIFY(!error.isEmpty());

    QVERIFY(!MpvCommandEncoder::encode(
        MpvSeekRequest{std::numeric_limits<double>::quiet_NaN(), MpvSeekMode::Absolute},
        &error).has_value());
    QVERIFY(!error.isEmpty());

    QVERIFY(!MpvCommandEncoder::encode(MpvVolumeRequest{-1.0}, &error).has_value());
    QVERIFY(!error.isEmpty());

    QVERIFY(!MpvCommandEncoder::encode(MpvSpeedRequest{0.0}, &error).has_value());
    QVERIFY(!error.isEmpty());

    QVERIFY(!MpvCommandEncoder::encode(
        MpvTrackSelectionRequest{MpvTrackSelectionKind::Audio, std::nullopt},
        &error).has_value());
    QVERIFY(!error.isEmpty());

    QVERIFY(!MpvCommandEncoder::encode(
        MpvTrackSelectionRequest{MpvTrackSelectionKind::Subtitle, qint64{0}},
        &error).has_value());
    QVERIFY(!error.isEmpty());
}

void MpvCommandExecutorTest::executorRequiresInitializedHandle()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvCommandExecutor executor(*handle);
    QVERIFY(!executor.submit(1, MpvPlayRequest{}, &error));
    QVERIFY(!error.isEmpty());
}

void MpvCommandExecutorTest::executorRejectsSubmissionFromAnotherThread()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvCommandExecutor executor(*handle);
    bool submitted = true;
    QString workerError;
    std::thread worker([&executor, &submitted, &workerError] {
        submitted = executor.submit(2, MpvPauseRequest{}, &workerError);
    });
    worker.join();

    QVERIFY(!submitted);
    QVERIFY(!workerError.isEmpty());
}

void MpvCommandExecutorTest::asyncSubmissionRepliesForMvpCommands()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvEventLoop eventLoop(*handle);
    QSignalSpy eventSpy(&eventLoop, &MpvEventLoop::eventDecoded);
    QVERIFY2(eventLoop.start(&error), qPrintable(error));

    MpvCommandExecutor executor(*handle);
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString missingMedia = temporaryDirectory.filePath(QStringLiteral("missing-media.wav"));
    const QString missingSubtitle = temporaryDirectory.filePath(QStringLiteral("missing-subtitle.srt"));

    struct PendingRequest final
    {
        quint64 id;
        MpvCommandRequest request;
    };

    const QList<PendingRequest> requests{
        {5001, MpvPauseRequest{}},
        {5002, MpvLoadRequest{missingMedia}},
        {5003, MpvSeekRequest{0.0, MpvSeekMode::Absolute}},
        {5004, MpvVolumeRequest{65.0}},
        {5005, MpvMuteRequest{true}},
        {5006, MpvSpeedRequest{1.1}},
        {5007, MpvPlayRequest{}},
        {5008, MpvStopRequest{}},
        {5009, MpvTrackSelectionRequest{MpvTrackSelectionKind::Subtitle, std::nullopt}},
        {5010, MpvExternalSubtitleRequest{missingSubtitle}},
    };

    QSet<quint64> expectedReplies;
    for (const PendingRequest& entry : requests) {
        expectedReplies.insert(entry.id);
        QVERIFY2(executor.submit(entry.id, entry.request, &error), qPrintable(error));
    }

    QTRY_VERIFY_WITH_TIMEOUT(containsCommandReplies(eventSpy, expectedReplies), 5000);

    eventLoop.stop();
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvCommandExecutorTest)
#include "mpv_command_executor_test.moc"
