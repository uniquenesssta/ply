#include "presentation/viewmodels/player/hud/hud_message_queue.h"

#include <QtTest>

#include <limits>

namespace player::presentation {

class HudMessageQueueTest final : public QObject
{
    Q_OBJECT

private slots:
    void volumeMessageClampsAndRounds();
    void rapidVolumeUpdatesCoalesce();
    void muteUpdatesCurrentVolumeMessage();
    void differentKindsQueueLatestValue();
    void invalidMessagesAreIgnored();
    void clearStopsCurrentAndPendingMessages();
};

void HudMessageQueueTest::volumeMessageClampsAndRounds()
{
    HudMessageQueue queue;

    queue.showVolume(120.4, false);

    QVERIFY(queue.visible());
    QCOMPARE(queue.messageKey(), QStringLiteral("volume"));
    QCOMPARE(queue.valueText(), QStringLiteral("100%"));
}

void HudMessageQueueTest::rapidVolumeUpdatesCoalesce()
{
    HudMessageQueue queue;

    for (int percent = 10; percent <= 90; percent += 10) {
        queue.showVolume(static_cast<double>(percent), false);
    }

    QCOMPARE(queue.messageKey(), QStringLiteral("volume"));
    QCOMPARE(queue.valueText(), QStringLiteral("90%"));
    QTRY_VERIFY_WITH_TIMEOUT(
        !queue.visible(),
        HudMessageQueue::holdDurationMs() + 700);
}

void HudMessageQueueTest::muteUpdatesCurrentVolumeMessage()
{
    HudMessageQueue queue;

    queue.showVolume(42.0, false);
    queue.showVolume(42.0, true);

    QVERIFY(queue.visible());
    QCOMPARE(queue.messageKey(), QStringLiteral("muted"));
    QCOMPARE(queue.valueText(), QString{});
    QTRY_VERIFY_WITH_TIMEOUT(
        !queue.visible(),
        HudMessageQueue::holdDurationMs() + 700);
}

void HudMessageQueueTest::differentKindsQueueLatestValue()
{
    HudMessageQueue queue;

    queue.showVolume(35.0, false);
    queue.showSeek(QStringLiteral("00:10"));
    queue.showSeek(QStringLiteral("00:11"));

    QCOMPARE(queue.messageKey(), QStringLiteral("volume"));
    QCOMPARE(queue.valueText(), QStringLiteral("35%"));

    QTRY_COMPARE_WITH_TIMEOUT(
        queue.messageKey(),
        QStringLiteral("seek"),
        HudMessageQueue::holdDurationMs() + 700);
    QCOMPARE(queue.valueText(), QStringLiteral("00:11"));

    QTRY_VERIFY_WITH_TIMEOUT(
        !queue.visible(),
        HudMessageQueue::holdDurationMs() + 700);
}

void HudMessageQueueTest::invalidMessagesAreIgnored()
{
    HudMessageQueue queue;

    queue.showVolume(std::numeric_limits<double>::quiet_NaN(), false);
    queue.showSeek(QStringLiteral("   "));

    QVERIFY(!queue.visible());
    QCOMPARE(queue.messageKey(), QString{});
    QCOMPARE(queue.valueText(), QString{});
}

void HudMessageQueueTest::clearStopsCurrentAndPendingMessages()
{
    HudMessageQueue queue;

    queue.showVolume(50.0, false);
    queue.showSeek(QStringLiteral("01:23"));
    queue.clear();

    QVERIFY(!queue.visible());
    QCOMPARE(queue.messageKey(), QString{});
    QCOMPARE(queue.valueText(), QString{});

    QTest::qWait(HudMessageQueue::holdDurationMs() + 100);
    QVERIFY(!queue.visible());
}

} // namespace player::presentation

QTEST_GUILESS_MAIN(player::presentation::HudMessageQueueTest)
#include "hud_message_queue_test.moc"
