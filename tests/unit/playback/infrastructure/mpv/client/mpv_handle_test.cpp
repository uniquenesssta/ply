#include "playback/infrastructure/mpv/client/mpv_handle.h"

#include <mpv/client.h>

#include <QString>
#include <QtTest>

namespace player::playback::mpv {

class MpvHandleTest final : public QObject
{
    Q_OBJECT

private slots:
    void createOwnsHandle();
    void initializeAndClose();
    void initializeAfterCloseFailsSafely();
    void closeIsIdempotent();
    void repeatedCreateDestroyCycles();
};

void MpvHandleTest::createOwnsHandle()
{
    QString error;
    auto handle = MpvHandle::create(&error);

    QVERIFY2(handle != nullptr, qPrintable(error));
    QVERIFY(handle->isOpen());
    QVERIFY(!handle->isInitialized());
    QVERIFY(handle->nativeHandle() != nullptr);
}

void MpvHandleTest::initializeAndClose()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    const int configResult = mpv_set_option_string(handle->nativeHandle(), "config", "no");
    QVERIFY2(
        configResult >= 0,
        qPrintable(QStringLiteral("Unable to disable user mpv config for lifecycle test: %1")
                       .arg(QString::fromUtf8(mpv_error_string(configResult)))));

    QVERIFY2(handle->initialize(&error), qPrintable(error));
    QVERIFY(handle->isOpen());
    QVERIFY(handle->isInitialized());

    error.clear();
    QVERIFY2(handle->initialize(&error), qPrintable(error));

    handle->close();
    QVERIFY(!handle->isOpen());
    QVERIFY(!handle->isInitialized());
    QVERIFY(handle->nativeHandle() == nullptr);
}

void MpvHandleTest::initializeAfterCloseFailsSafely()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    handle->close();

    QVERIFY(!handle->initialize(&error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!handle->isOpen());
    QVERIFY(!handle->isInitialized());
}

void MpvHandleTest::closeIsIdempotent()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    handle->close();
    handle->close();

    QVERIFY(!handle->isOpen());
    QVERIFY(!handle->isInitialized());
}

void MpvHandleTest::repeatedCreateDestroyCycles()
{
    for (int cycle = 0; cycle < 100; ++cycle) {
        QString error;
        auto handle = MpvHandle::create(&error);
        QVERIFY2(handle != nullptr, qPrintable(error));
        QVERIFY(handle->isOpen());
        handle->close();
        QVERIFY(!handle->isOpen());
    }
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvHandleTest)
#include "mpv_handle_test.moc"
