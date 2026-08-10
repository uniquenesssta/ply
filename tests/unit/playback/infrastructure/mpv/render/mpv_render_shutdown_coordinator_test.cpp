#include "playback/infrastructure/mpv/render/mpv_render_shutdown_coordinator.h"

#include <QtTest>

#include <chrono>

namespace {

using player::playback::infrastructure::mpv::render::MpvRenderShutdownCoordinator;
using namespace std::chrono_literals;

class MpvRenderShutdownCoordinatorTest final : public QObject
{
    Q_OBJECT

private slots:
    void runningCoordinatorAcceptsRenderSections();
    void shutdownWaitsForActiveRenderSection();
    void shutdownIsIdempotentAndRejectsNewSections();
};

void MpvRenderShutdownCoordinatorTest::runningCoordinatorAcceptsRenderSections()
{
    MpvRenderShutdownCoordinator coordinator;

    const auto initial = coordinator.snapshot();
    QVERIFY(!initial.shutdownRequested);
    QVERIFY(initial.activeRenderSections == 0);
    QVERIFY(initial.liveRenderContexts == 0);
    QVERIFY(!initial.renderReleased);

    {
        auto renderSection = coordinator.tryEnterRenderSection();
        QVERIFY(static_cast<bool>(renderSection));

        const auto active = coordinator.snapshot();
        QVERIFY(active.activeRenderSections == 1);
        QVERIFY(!active.renderReleased);
    }

    QVERIFY(coordinator.snapshot().activeRenderSections == 0);
}

void MpvRenderShutdownCoordinatorTest::shutdownWaitsForActiveRenderSection()
{
    MpvRenderShutdownCoordinator coordinator;
    auto renderSection = coordinator.tryEnterRenderSection();
    QVERIFY(static_cast<bool>(renderSection));

    QVERIFY(coordinator.beginShutdown());
    QVERIFY(coordinator.isShutdownRequested());
    QVERIFY(!coordinator.waitForRenderRelease(10ms));

    renderSection = MpvRenderShutdownCoordinator::RenderSection{};

    QVERIFY(coordinator.waitForRenderRelease(100ms));
    const auto released = coordinator.snapshot();
    QVERIFY(released.renderReleased);
    QVERIFY(released.activeRenderSections == 0);
}

void MpvRenderShutdownCoordinatorTest::shutdownIsIdempotentAndRejectsNewSections()
{
    MpvRenderShutdownCoordinator coordinator;

    QVERIFY(coordinator.beginShutdown());
    QVERIFY(!coordinator.beginShutdown());

    auto renderSection = coordinator.tryEnterRenderSection();
    QVERIFY(!static_cast<bool>(renderSection));
    QVERIFY(coordinator.waitForRenderRelease(0ms));
    QVERIFY(coordinator.snapshot().renderReleased);
}

} // namespace

QTEST_GUILESS_MAIN(MpvRenderShutdownCoordinatorTest)
#include "mpv_render_shutdown_coordinator_test.moc"
