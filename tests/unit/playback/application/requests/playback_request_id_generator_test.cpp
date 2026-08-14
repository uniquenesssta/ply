#include "playback/application/requests/playback_request_id_generator.h"

#include <QSet>
#include <QtTest>

namespace player::playback::application {

class PlaybackRequestIdGeneratorTest final : public QObject
{
    Q_OBJECT

private slots:
    void generatedIdsAreValidAndUnique();
};

void PlaybackRequestIdGeneratorTest::generatedIdsAreValidAndUnique()
{
    PlaybackRequestIdGenerator generator;
    QSet<quint64> observed;

    constexpr int sampleCount = 1024;
    for (int index = 0; index < sampleCount; ++index) {
        const player::ids::RequestId id = generator.next();
        QVERIFY(id.isValid());
        QVERIFY(!observed.contains(id.value()));
        observed.insert(id.value());
    }

    QCOMPARE(observed.size(), sampleCount);
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::PlaybackRequestIdGeneratorTest)
#include "playback_request_id_generator_test.moc"
