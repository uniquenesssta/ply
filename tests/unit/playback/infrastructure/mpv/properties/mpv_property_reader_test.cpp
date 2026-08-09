#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/properties/mpv_property_reader.h"

#include <mpv/client.h>

#include <QString>
#include <QThread>
#include <QtTest>

#include <cmath>
#include <memory>
#include <variant>

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

} // namespace

class MpvPropertyReaderTest final : public QObject
{
    Q_OBJECT

private slots:
    void requiresInitializedHandle();
    void rejectsCrossThreadRead();
    void readsCurrentScalarValues();
};

void MpvPropertyReaderTest::requiresInitializedHandle()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvPropertyReader reader(*handle);
    const auto value = reader.read(MpvPropertyId::Pause, &error);
    QVERIFY(!value.has_value());
    QVERIFY(!error.isEmpty());
}

void MpvPropertyReaderTest::rejectsCrossThreadRead()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvPropertyReader reader(*handle);
    bool rejected = false;
    QString diagnostic;

    std::unique_ptr<QThread> worker(QThread::create([&reader, &rejected, &diagnostic]() {
        const auto value = reader.read(MpvPropertyId::Pause, &diagnostic);
        rejected = !value.has_value() && !diagnostic.isEmpty();
    }));
    QVERIFY(worker != nullptr);
    worker->start();
    QVERIFY(worker->wait(5000));
    QVERIFY(rejected);
}

void MpvPropertyReaderTest::readsCurrentScalarValues()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvPropertyReader reader(*handle);

    int result = mpv_set_property_string(handle->nativeHandle(), "pause", "yes");
    QVERIFY2(result >= 0, mpv_error_string(result));

    const auto pause = reader.read(MpvPropertyId::Pause, &error);
    QVERIFY2(pause.has_value(), qPrintable(error));
    const bool* pauseValue = std::get_if<bool>(&pause->value);
    QVERIFY(pauseValue != nullptr);
    QVERIFY(*pauseValue);

    result = mpv_set_property_string(handle->nativeHandle(), "volume", "42.5");
    QVERIFY2(result >= 0, mpv_error_string(result));

    const auto volume = reader.read(MpvPropertyId::Volume, &error);
    QVERIFY2(volume.has_value(), qPrintable(error));
    const double* volumeValue = std::get_if<double>(&volume->value);
    QVERIFY(volumeValue != nullptr);
    QVERIFY(std::abs(*volumeValue - 42.5) < 0.0001);
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvPropertyReaderTest)
#include "mpv_property_reader_test.moc"
