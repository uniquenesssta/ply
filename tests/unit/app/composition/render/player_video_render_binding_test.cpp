#include "app/composition/render/player_video_render_binding.h"

#include "playback/application/session/playback_session_thread.h"
#include "playback/infrastructure/mpv/render/mpv_render_shutdown_coordinator.h"
#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QGuiApplication>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtTest>

#include <cstddef>

namespace player::app {
namespace {

using player::playback::application::PlaybackSessionThread;
using player::playback::infrastructure::mpv::render::MpvVideoItem;

bool publishRenderCore(PlaybackSessionThread& thread, mpv_handle* core)
{
    return QMetaObject::invokeMethod(
        &thread,
        "renderCoreReady",
        Qt::DirectConnection,
        Q_ARG(quintptr, reinterpret_cast<quintptr>(core)));
}

MpvVideoItem* addVideoItem(QObject& root)
{
    auto* item = new MpvVideoItem();
    item->setObjectName(QStringLiteral("mpvVideoItem"));
    item->setParent(&root);
    return item;
}

} // namespace

class PlayerVideoRenderBindingTest final : public QObject
{
    Q_OBJECT

private slots:
    void bindsCoreThatArrivesBeforeQmlRoot();
    void bindsCoreThatArrivesAfterQmlRoot();
    void rejectsRootWithoutVideoOutput();
};

void PlayerVideoRenderBindingTest::bindsCoreThatArrivesBeforeQmlRoot()
{
    PlaybackSessionThread thread;
    PlayerVideoRenderBinding binding(thread);
    QString error;
    QVERIFY2(binding.resetForStart(&error), qPrintable(error));

    std::byte token{};
    auto* fakeCore = reinterpret_cast<mpv_handle*>(&token);
    QVERIFY(publishRenderCore(thread, fakeCore));

    QObject root;
    MpvVideoItem* item = addVideoItem(root);
    QVERIFY2(binding.attachRoot(&root, &error), qPrintable(error));
    QCOMPARE(item->renderCoreHandle(), fakeCore);

    const auto coordinator = item->renderShutdownCoordinator();
    QVERIFY(coordinator != nullptr);
    binding.beginShutdown();
    QVERIFY(item->renderCoreHandle() == nullptr);
    QVERIFY(coordinator->snapshot().renderReleased);
    QVERIFY2(binding.waitForRenderRelease(&error), qPrintable(error));
}

void PlayerVideoRenderBindingTest::bindsCoreThatArrivesAfterQmlRoot()
{
    PlaybackSessionThread thread;
    PlayerVideoRenderBinding binding(thread);
    QString error;
    QVERIFY2(binding.resetForStart(&error), qPrintable(error));

    QObject root;
    MpvVideoItem* item = addVideoItem(root);
    QVERIFY2(binding.attachRoot(&root, &error), qPrintable(error));
    QVERIFY(item->renderCoreHandle() == nullptr);

    std::byte token{};
    auto* fakeCore = reinterpret_cast<mpv_handle*>(&token);
    QVERIFY(publishRenderCore(thread, fakeCore));
    QCOMPARE(item->renderCoreHandle(), fakeCore);

    const auto coordinator = item->renderShutdownCoordinator();
    QVERIFY(coordinator != nullptr);
    binding.beginShutdown();
    QVERIFY(coordinator->snapshot().renderReleased);
    QVERIFY2(binding.waitForRenderRelease(&error), qPrintable(error));
}

void PlayerVideoRenderBindingTest::rejectsRootWithoutVideoOutput()
{
    PlaybackSessionThread thread;
    PlayerVideoRenderBinding binding(thread);
    QString error;
    QVERIFY2(binding.resetForStart(&error), qPrintable(error));

    QObject root;
    QVERIFY(!binding.attachRoot(&root, &error));
    QVERIFY(!error.isEmpty());

    binding.beginShutdown();
    QVERIFY(binding.waitForRenderRelease(&error));
}

} // namespace player::app

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::app::PlayerVideoRenderBindingTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "player_video_render_binding_test.moc"
