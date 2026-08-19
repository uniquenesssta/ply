#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/properties/mpv_property_observer.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <mpv/client.h>

#include <QElapsedTimer>
#include <QSet>
#include <QString>
#include <QtTest>

#include <cmath>
#include <memory>
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

bool waitForBoolProperty(
    MpvHandle& handle,
    const MpvPropertyObserver& observer,
    MpvPropertyId propertyId,
    bool expectedValue,
    QString* lastError)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 2000) {
        mpv_event* event = mpv_wait_event(handle.nativeHandle(), 0.05);
        if (event == nullptr || event->event_id == MPV_EVENT_NONE) {
            continue;
        }
        if (event->event_id != MPV_EVENT_PROPERTY_CHANGE || event->data == nullptr) {
            continue;
        }

        const auto* property = static_cast<const mpv_event_property*>(event->data);
        auto change = observer.decode(
            static_cast<quint64>(event->reply_userdata),
            static_cast<int>(property->format),
            property->data,
            lastError);
        if (!change.has_value() || change->id != propertyId) {
            continue;
        }

        const bool* value = std::get_if<bool>(&change->value);
        if (value != nullptr && *value == expectedValue) {
            return true;
        }
    }

    return false;
}

bool waitForDoubleProperty(
    MpvHandle& handle,
    const MpvPropertyObserver& observer,
    MpvPropertyId propertyId,
    double expectedValue,
    QString* lastError)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 2000) {
        mpv_event* event = mpv_wait_event(handle.nativeHandle(), 0.05);
        if (event == nullptr || event->event_id == MPV_EVENT_NONE) {
            continue;
        }
        if (event->event_id != MPV_EVENT_PROPERTY_CHANGE || event->data == nullptr) {
            continue;
        }

        const auto* property = static_cast<const mpv_event_property*>(event->data);
        auto change = observer.decode(
            static_cast<quint64>(event->reply_userdata),
            static_cast<int>(property->format),
            property->data,
            lastError);
        if (!change.has_value() || change->id != propertyId) {
            continue;
        }

        const double* value = std::get_if<double>(&change->value);
        if (value != nullptr && std::abs(*value - expectedValue) < 0.0001) {
            return true;
        }
    }

    return false;
}

} // namespace

class MpvPropertyObserverTest final : public QObject
{
    Q_OBJECT

private slots:
    void registryOwnsCorePropertyDefinitions();
    void observerRequiresInitializedHandle();
    void observerRejectsStartFromAnotherThread();
    void observesAndDecodesCoreProperties();
    void decodeHandlesUnavailableUnexpectedAndNodeFormats();
};

void MpvPropertyObserverTest::registryOwnsCorePropertyDefinitions()
{
    const QList<MpvPropertyDefinition>& definitions = MpvPropertyRegistry::coreDefinitions();
    QCOMPARE(definitions.size(), qsizetype{23});

    QSet<int> ids;
    QSet<quint64> observationIds;
    QSet<QByteArray> names;
    for (const MpvPropertyDefinition& definition : definitions) {
        QVERIFY(!definition.name.isEmpty());
        QVERIFY(definition.observationId != 0);
        QVERIFY(!ids.contains(static_cast<int>(definition.id)));
        QVERIFY(!observationIds.contains(definition.observationId));
        QVERIFY(!names.contains(definition.name));
        ids.insert(static_cast<int>(definition.id));
        observationIds.insert(definition.observationId);
        names.insert(definition.name);
    }

    const MpvPropertyDefinition* position = MpvPropertyRegistry::findById(MpvPropertyId::Position);
    QVERIFY(position != nullptr);
    QCOMPARE(position->name, QByteArrayLiteral("time-pos"));
    QCOMPARE(position->format, MpvPropertyFormat::Double);

    const MpvPropertyDefinition* trackList = MpvPropertyRegistry::findById(MpvPropertyId::TrackList);
    const MpvPropertyDefinition* chapterList = MpvPropertyRegistry::findById(MpvPropertyId::ChapterList);
    const MpvPropertyDefinition* subtitleDelay = MpvPropertyRegistry::findById(MpvPropertyId::SubtitleDelay);
    QVERIFY(trackList != nullptr);
    QVERIFY(chapterList != nullptr);
    QVERIFY(subtitleDelay != nullptr);
    QCOMPARE(trackList->format, MpvPropertyFormat::Node);
    QCOMPARE(chapterList->format, MpvPropertyFormat::Node);
    QCOMPARE(subtitleDelay->name, QByteArrayLiteral("sub-delay"));
    QCOMPARE(subtitleDelay->format, MpvPropertyFormat::Double);
}

void MpvPropertyObserverTest::observerRequiresInitializedHandle()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvPropertyObserver observer(*handle);
    QVERIFY(!observer.start(&error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!observer.isObserving());
}

void MpvPropertyObserverTest::observerRejectsStartFromAnotherThread()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvPropertyObserver observer(*handle);
    bool started = true;
    QString workerError;
    std::thread worker([&observer, &started, &workerError] {
        started = observer.start(&workerError);
    });
    worker.join();

    QVERIFY(!started);
    QVERIFY(!workerError.isEmpty());
    QVERIFY(!observer.isObserving());
}

void MpvPropertyObserverTest::observesAndDecodesCoreProperties()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvPropertyObserver observer(*handle);
    QVERIFY2(observer.start(&error), qPrintable(error));
    QVERIFY(observer.isObserving());
    QVERIFY2(observer.start(&error), qPrintable(error));

    int result = mpv_set_property_string(handle->nativeHandle(), "pause", "yes");
    QVERIFY2(result >= 0, mpv_error_string(result));
    QVERIFY2(waitForBoolProperty(*handle, observer, MpvPropertyId::Pause, true, &error), qPrintable(error));

    result = mpv_set_property_string(handle->nativeHandle(), "volume", "42.5");
    QVERIFY2(result >= 0, mpv_error_string(result));
    QVERIFY2(waitForDoubleProperty(*handle, observer, MpvPropertyId::Volume, 42.5, &error), qPrintable(error));

    result = mpv_set_property_string(handle->nativeHandle(), "mute", "yes");
    QVERIFY2(result >= 0, mpv_error_string(result));
    QVERIFY2(waitForBoolProperty(*handle, observer, MpvPropertyId::Mute, true, &error), qPrintable(error));

    result = mpv_set_property_string(handle->nativeHandle(), "speed", "1.25");
    QVERIFY2(result >= 0, mpv_error_string(result));
    QVERIFY2(waitForDoubleProperty(*handle, observer, MpvPropertyId::Speed, 1.25, &error), qPrintable(error));

    result = mpv_set_property_string(handle->nativeHandle(), "sub-delay", "0.25");
    QVERIFY2(result >= 0, mpv_error_string(result));
    QVERIFY2(waitForDoubleProperty(*handle, observer, MpvPropertyId::SubtitleDelay, 0.25, &error), qPrintable(error));

    QVERIFY2(observer.stop(&error), qPrintable(error));
    QVERIFY(!observer.isObserving());
    QVERIFY2(observer.stop(&error), qPrintable(error));
}

void MpvPropertyObserverTest::decodeHandlesUnavailableUnexpectedAndNodeFormats()
{
    QString error;
    auto handle = createInitializedHandle(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    MpvPropertyObserver observer(*handle);
    const MpvPropertyDefinition* pause = MpvPropertyRegistry::findById(MpvPropertyId::Pause);
    const MpvPropertyDefinition* trackList = MpvPropertyRegistry::findById(MpvPropertyId::TrackList);
    QVERIFY(pause != nullptr);
    QVERIFY(trackList != nullptr);

    auto unavailable = observer.decode(pause->observationId, MPV_FORMAT_NONE, nullptr, &error);
    QVERIFY2(unavailable.has_value(), qPrintable(error));
    QVERIFY(std::holds_alternative<std::monostate>(unavailable->value));

    double wrongValue = 1.0;
    auto wrongFormat = observer.decode(pause->observationId, MPV_FORMAT_DOUBLE, &wrongValue, &error);
    QVERIFY(!wrongFormat.has_value());
    QVERIFY(!error.isEmpty());

    int flagValue = 1;
    auto unknown = observer.decode(999999, MPV_FORMAT_FLAG, &flagValue, &error);
    QVERIFY(!unknown.has_value());
    QVERIFY(!error.isEmpty());

    mpv_node_list emptyList{};
    mpv_node node{};
    node.format = MPV_FORMAT_NODE_ARRAY;
    node.u.list = &emptyList;
    auto nodeChange = observer.decode(trackList->observationId, MPV_FORMAT_NODE, &node, &error);
    QVERIFY2(nodeChange.has_value(), qPrintable(error));
    const QVariant* nodeValue = std::get_if<QVariant>(&nodeChange->value);
    QVERIFY(nodeValue != nullptr);
    QVERIFY(nodeValue->toList().isEmpty());
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvPropertyObserverTest)
#include "mpv_property_observer_test.moc"
