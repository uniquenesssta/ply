#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/events/mpv_event_loop.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"

#include <mpv/client.h>

#include <QSignalSpy>
#include <QString>
#include <QThread>
#include <QtTest>

#include <memory>

namespace player::playback::mpv {
namespace {

bool containsEventId(const QSignalSpy& spy, int eventId)
{
    for (const QList<QVariant>& arguments : spy) {
        if (!arguments.isEmpty() && arguments.first().toInt() == eventId) {
            return true;
        }
    }
    return false;
}

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

} // namespace

class MpvEventLoopTest final : public QObject
{
    Q_OBJECT

private slots:
    void requiresInitializedHandle();
    void drainsWakeupsOnOwningQtThread();
    void stopIsIdempotentAndSuppressesFurtherDrain();
    void destructorDisablesWakeupBeforeTargetDestruction();
};

void MpvEventLoopTest::requiresInitializedHandle()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvEventLoop eventLoop(*handle);
    QVERIFY(!eventLoop.start(&error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!eventLoop.isRunning());
}

void MpvEventLoopTest::drainsWakeupsOnOwningQtThread()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvEventLoop eventLoop(*handle);
    QSignalSpy spy(&eventLoop, &MpvEventLoop::eventDrained);
    QThread* deliveryThread = nullptr;
    connect(
        &eventLoop,
        &MpvEventLoop::eventDrained,
        this,
        [&deliveryThread](int, quint64, int) {
            deliveryThread = QThread::currentThread();
        });

    QVERIFY2(eventLoop.start(&error), qPrintable(error));
    QVERIFY(eventLoop.isRunning());
    QVERIFY2(eventLoop.start(&error), qPrintable(error));

    constexpr uint64_t kObservationId = 1001;
    const int observeResult = mpv_observe_property(
        handle->nativeHandle(),
        kObservationId,
        "pause",
        MPV_FORMAT_FLAG);
    QVERIFY2(
        observeResult >= 0,
        qPrintable(QStringLiteral("mpv_observe_property failed: %1")
                       .arg(QString::fromUtf8(mpv_error_string(observeResult)))));

    QTRY_VERIFY_WITH_TIMEOUT(containsEventId(spy, MPV_EVENT_PROPERTY_CHANGE), 2000);
    QCOMPARE(deliveryThread, QThread::currentThread());

    const int unobserveResult = mpv_unobserve_property(handle->nativeHandle(), kObservationId);
    QVERIFY(unobserveResult >= 0);

    eventLoop.stop();
    QVERIFY(!eventLoop.isRunning());
}

void MpvEventLoopTest::stopIsIdempotentAndSuppressesFurtherDrain()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvEventLoop eventLoop(*handle);
    QSignalSpy spy(&eventLoop, &MpvEventLoop::eventDrained);
    QVERIFY2(eventLoop.start(&error), qPrintable(error));

    constexpr uint64_t kObservationId = 1002;
    const int observeResult = mpv_observe_property(
        handle->nativeHandle(),
        kObservationId,
        "pause",
        MPV_FORMAT_FLAG);
    QVERIFY(observeResult >= 0);
    QTRY_VERIFY_WITH_TIMEOUT(containsEventId(spy, MPV_EVENT_PROPERTY_CHANGE), 2000);

    spy.clear();
    eventLoop.stop();
    eventLoop.stop();

    const int setResult = mpv_set_property_string(handle->nativeHandle(), "pause", "yes");
    QVERIFY2(
        setResult >= 0,
        qPrintable(QStringLiteral("mpv_set_property_string failed: %1")
                       .arg(QString::fromUtf8(mpv_error_string(setResult)))));

    QTest::qWait(100);
    QCOMPARE(spy.count(), 0);

    const int unobserveResult = mpv_unobserve_property(handle->nativeHandle(), kObservationId);
    QVERIFY(unobserveResult >= 0);
}

void MpvEventLoopTest::destructorDisablesWakeupBeforeTargetDestruction()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    constexpr uint64_t kObservationId = 1003;
    {
        auto eventLoop = std::make_unique<MpvEventLoop>(*handle);
        QSignalSpy spy(eventLoop.get(), &MpvEventLoop::eventDrained);
        QVERIFY2(eventLoop->start(&error), qPrintable(error));

        const int observeResult = mpv_observe_property(
            handle->nativeHandle(),
            kObservationId,
            "mute",
            MPV_FORMAT_FLAG);
        QVERIFY(observeResult >= 0);
        QTRY_VERIFY_WITH_TIMEOUT(containsEventId(spy, MPV_EVENT_PROPERTY_CHANGE), 2000);
    }

    const int setResult = mpv_set_property_string(handle->nativeHandle(), "mute", "yes");
    QVERIFY(setResult >= 0);
    QTest::qWait(100);

    const int unobserveResult = mpv_unobserve_property(handle->nativeHandle(), kObservationId);
    QVERIFY(unobserveResult >= 0);
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvEventLoopTest)
#include "mpv_event_loop_test.moc"
