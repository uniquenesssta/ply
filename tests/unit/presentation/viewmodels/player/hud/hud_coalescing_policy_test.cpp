#include "presentation/viewmodels/player/hud/hud_message_queue.h"

#include <QtTest>

namespace player::presentation {

class HudCoalescingPolicyTest final : public QObject
{
    Q_OBJECT

private slots:
    void speedBurstCoalescesToLatestValue();
    void trackChangeStaysIndependentFromVolume();
    void importantFeedbackPreemptsAndCannotBeOverwritten();
    void boundedPendingDropsOldestTransientCategory();
    void emptyFutureMessagesAreIgnored();
};

void HudCoalescingPolicyTest::speedBurstCoalescesToLatestValue()
{
    HudMessageQueue queue;

    queue.showSpeed(QStringLiteral("1.05×"));
    queue.showSpeed(QStringLiteral("1.10×"));
    queue.showSpeed(QStringLiteral("1.25×"));

    QVERIFY(queue.visible());
    QCOMPARE(queue.messageKey(), QStringLiteral("speed"));
    QCOMPARE(queue.valueText(), QStringLiteral("1.25×"));
}

void HudCoalescingPolicyTest::trackChangeStaysIndependentFromVolume()
{
    HudMessageQueue queue;

    queue.showVolume(30.0, false);
    queue.showTrackChange(QStringLiteral("English"));
    queue.showVolume(40.0, false);

    QCOMPARE(queue.messageKey(), QStringLiteral("volume"));
    QCOMPARE(queue.valueText(), QStringLiteral("40%"));

    QTRY_COMPARE_WITH_TIMEOUT(
        queue.messageKey(),
        QStringLiteral("track"),
        HudMessageQueue::holdDurationMs() + 700);
    QCOMPARE(queue.valueText(), QStringLiteral("English"));
}

void HudCoalescingPolicyTest::importantFeedbackPreemptsAndCannotBeOverwritten()
{
    HudMessageQueue queue;

    queue.showVolume(25.0, false);
    queue.showTrackChange(QStringLiteral("Japanese"));
    queue.showSeekFailure();

    QCOMPARE(queue.messageKey(), QStringLiteral("seekFailed"));
    QCOMPARE(queue.valueText(), QString{});

    queue.showVolume(60.0, false);
    queue.showSeek(QStringLiteral("00:45"));

    QCOMPARE(queue.messageKey(), QStringLiteral("seekFailed"));

    QTRY_COMPARE_WITH_TIMEOUT(
        queue.messageKey(),
        QStringLiteral("volume"),
        HudMessageQueue::holdDurationMs() + 700);
    QCOMPARE(queue.valueText(), QStringLiteral("60%"));
}

void HudCoalescingPolicyTest::boundedPendingDropsOldestTransientCategory()
{
    HudMessageQueue queue;

    queue.showSeekFailure();
    queue.showVolume(10.0, false);
    queue.showSeek(QStringLiteral("00:10"));
    queue.showSpeed(QStringLiteral("1.25×"));
    queue.showTrackChange(QStringLiteral("English"));

    QCOMPARE(queue.messageKey(), QStringLiteral("seekFailed"));

    QTRY_COMPARE_WITH_TIMEOUT(
        queue.messageKey(),
        QStringLiteral("seek"),
        HudMessageQueue::holdDurationMs() + 700);
    QCOMPARE(queue.valueText(), QStringLiteral("00:10"));
}

void HudCoalescingPolicyTest::emptyFutureMessagesAreIgnored()
{
    HudMessageQueue queue;

    queue.showSpeed(QStringLiteral("   "));
    queue.showTrackChange(QStringLiteral("\t"));

    QVERIFY(!queue.visible());
    QCOMPARE(queue.messageKey(), QString{});
    QCOMPARE(queue.valueText(), QString{});
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::HudCoalescingPolicyTest)
#include "hud_coalescing_policy_test.moc"
