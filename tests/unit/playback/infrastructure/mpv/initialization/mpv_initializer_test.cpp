#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/initialization/mpv_option_profile.h"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QtTest>

namespace player::playback::mpv {
namespace {

QByteArray optionValue(const MpvOptionProfile& profile, const QByteArray& name)
{
    for (const MpvOption& option : profile.options()) {
        if (option.name == name) {
            return option.value;
        }
    }
    return {};
}

} // namespace

class MpvInitializerTest final : public QObject
{
    Q_OBJECT

private slots:
    void productProfileDisablesUserConfig();
    void productInitializationSucceeds();
    void repeatedProductInitializationIsIdempotent();
    void invalidOptionFailsAndClosesHandle();
    void closedHandleFailsSafely();
};

void MpvInitializerTest::productProfileDisablesUserConfig()
{
    const MpvOptionProfile profile = MpvOptionProfile::productDefaults();

    QCOMPARE(optionValue(profile, QByteArrayLiteral("config")), QByteArrayLiteral("no"));
}

void MpvInitializerTest::productInitializationSucceeds()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    QVERIFY2(MpvInitializer::initializeProduct(*handle, &error), qPrintable(error));
    QVERIFY(handle->isOpen());
    QVERIFY(handle->isInitialized());
}

void MpvInitializerTest::repeatedProductInitializationIsIdempotent()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    QVERIFY2(MpvInitializer::initializeProduct(*handle, &error), qPrintable(error));
    QVERIFY2(MpvInitializer::initializeProduct(*handle, &error), qPrintable(error));
    QVERIFY(handle->isInitialized());
}

void MpvInitializerTest::invalidOptionFailsAndClosesHandle()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    const MpvOptionProfile profile(QList<MpvOption>{
        {QByteArrayLiteral("__player_invalid_option__"), QByteArrayLiteral("1")},
    });

    QVERIFY(!MpvInitializer::initialize(*handle, profile, &error));
    QVERIFY(error.contains(QStringLiteral("__player_invalid_option__")));
    QVERIFY(!handle->isOpen());
    QVERIFY(!handle->isInitialized());
}

void MpvInitializerTest::closedHandleFailsSafely()
{
    QString error;
    auto handle = MpvHandle::create(&error);
    QVERIFY2(handle != nullptr, qPrintable(error));

    handle->close();

    QVERIFY(!MpvInitializer::initializeProduct(*handle, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!handle->isOpen());
    QVERIFY(!handle->isInitialized());
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvInitializerTest)
#include "mpv_initializer_test.moc"
