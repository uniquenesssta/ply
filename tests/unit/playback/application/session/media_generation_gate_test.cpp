#include "playback/application/session/backend/mpv_media_generation_attributor.h"
#include "playback/application/session/media_generation_gate.h"
#include "playback/domain/events/playback_event.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <QtTest/QTest>

#include <utility>
#include <variant>

namespace player::playback::application {
namespace {

using namespace player::playback::domain;
using namespace player::playback::mpv;

MpvEvent startFile(qint64 playlistEntryId)
{
    MpvEvent event;
    event.type = MpvEventType::StartFile;
    event.payload = MpvStartFileData{playlistEntryId};
    return event;
}

MpvEvent fileLoaded()
{
    MpvEvent event;
    event.type = MpvEventType::FileLoaded;
    return event;
}

MpvEvent endFile(
    qint64 playlistEntryId,
    MpvEndFileReason reason = MpvEndFileReason::Stop,
    qint64 playlistInsertId = 0,
    int playlistInsertNumEntries = 0)
{
    MpvEvent event;
    event.type = MpvEventType::EndFile;
    event.payload = MpvEndFileData{
        reason,
        playlistEntryId,
        playlistInsertId,
        playlistInsertNumEntries,
        static_cast<int>(reason)};
    return event;
}

MpvEvent propertyChange(MpvPropertyId id)
{
    MpvEvent event;
    event.type = MpvEventType::PropertyChange;
    event.payload = MpvPropertyChange{id, std::monostate{}};
    return event;
}

PlaybackEvent mediaEvent(PlaybackEventPayload payload, MediaGeneration generation)
{
    return PlaybackEvent{std::move(payload), generation};
}

} // namespace

class MediaGenerationTest final : public QObject
{
    Q_OBJECT

private slots:
    void gateRejectsStaleAndUnattributedMediaEvents();
    void attributorSeparatesOverlappingAAndBEvents();
    void replacementEndBeforeStartKeepsOldPropertyFence();
    void cancelledLoadDoesNotPoisonNextStartFile();
    void redirectEntriesKeepTheSameGeneration();
};

void MediaGenerationTest::gateRejectsStaleAndUnattributedMediaEvents()
{
    MediaGenerationGate gate;
    gate.activate(MediaGeneration{42});

    QVERIFY(!gate.accepts(mediaEvent(PositionChangedEvent{1.25}, MediaGeneration{41})));
    QVERIFY(!gate.accepts(mediaEvent(MediaLoadedEvent{}, MediaGeneration{41})));
    QVERIFY(!gate.accepts(mediaEvent(
        MediaEndedEvent{MediaEndReason::Eof},
        MediaGeneration{41})));

    QVERIFY(gate.accepts(mediaEvent(MediaPathChangedEvent{QStringLiteral("B.wav")}, MediaGeneration{42})));
    QVERIFY(gate.accepts(PlaybackEvent{VolumeChangedEvent{50.0}}));
    QVERIFY(!gate.accepts(PlaybackEvent{DurationChangedEvent{12.0}}));

    QCOMPARE(gate.currentGeneration().value(), quint64{42});
    QCOMPARE(gate.diagnostics().staleGenerationEventCount, quint64{3});
    QCOMPARE(gate.diagnostics().missingGenerationEventCount, quint64{1});
}

void MediaGenerationTest::attributorSeparatesOverlappingAAndBEvents()
{
    MpvMediaGenerationAttributor attributor;
    const MediaGeneration generationA{41};
    const MediaGeneration generationB{42};

    attributor.noteLoadSubmission(player::ids::RequestId{1001}, generationA);
    attributor.noteLoadSubmission(player::ids::RequestId{1002}, generationB);

    QCOMPARE(attributor.attribute(startFile(501)).value(), generationA.value());
    QCOMPARE(
        attributor.attribute(propertyChange(MpvPropertyId::Position)).value(),
        generationA.value());

    QCOMPARE(attributor.attribute(startFile(502)).value(), generationB.value());

    // Replacement fence: until B itself reaches FileLoaded, media properties stay
    // attributed to the previously established generation and are rejected by B's gate.
    const MediaGeneration transitionPosition = attributor.attribute(
        propertyChange(MpvPropertyId::Position));
    QCOMPARE(transitionPosition.value(), generationA.value());

    const MediaGeneration lateFileLoadedA = attributor.attribute(fileLoaded());
    QCOMPARE(lateFileLoadedA.value(), generationA.value());

    const MediaGeneration lateEndA = attributor.attribute(endFile(501));
    QCOMPARE(lateEndA.value(), generationA.value());

    const MediaGeneration lateTrackA = attributor.attribute(
        propertyChange(MpvPropertyId::TrackList));
    const MediaGeneration lateChapterA = attributor.attribute(
        propertyChange(MpvPropertyId::ChapterList));
    QCOMPARE(lateTrackA.value(), generationA.value());
    QCOMPARE(lateChapterA.value(), generationA.value());

    const MediaGeneration fileLoadedB = attributor.attribute(fileLoaded());
    QCOMPARE(fileLoadedB.value(), generationB.value());
    QCOMPARE(
        attributor.attribute(propertyChange(MpvPropertyId::Position)).value(),
        generationB.value());
    QCOMPARE(attributor.attribute(endFile(502)).value(), generationB.value());

    MediaGenerationGate gate;
    gate.activate(generationB);
    QVERIFY(!gate.accepts(mediaEvent(PositionChangedEvent{9.0}, transitionPosition)));
    QVERIFY(!gate.accepts(mediaEvent(MediaLoadedEvent{}, lateFileLoadedA)));
    QVERIFY(!gate.accepts(mediaEvent(
        MediaEndedEvent{MediaEndReason::Stopped},
        lateEndA)));
    QVERIFY(!gate.accepts(mediaEvent(MediaTitleChangedEvent{QStringLiteral("A")}, lateTrackA)));
    QVERIFY(!gate.accepts(mediaEvent(MediaPathChangedEvent{QStringLiteral("A")}, lateChapterA)));
    QVERIFY(gate.accepts(mediaEvent(MediaLoadedEvent{}, fileLoadedB)));
}

void MediaGenerationTest::replacementEndBeforeStartKeepsOldPropertyFence()
{
    MpvMediaGenerationAttributor attributor;
    const MediaGeneration generationA{71};
    const MediaGeneration generationB{72};

    attributor.noteLoadSubmission(player::ids::RequestId{4001}, generationA);
    attributor.noteLoadSubmission(player::ids::RequestId{4002}, generationB);

    QCOMPARE(attributor.attribute(startFile(901)).value(), generationA.value());
    QCOMPARE(attributor.attribute(fileLoaded()).value(), generationA.value());
    QCOMPARE(
        attributor.attribute(propertyChange(MpvPropertyId::Position)).value(),
        generationA.value());

    // B is already accepted by Session, but libmpv may end A before emitting StartFile for B.
    // Keep A as the property attribution fence until B itself reaches FileLoaded.
    QCOMPARE(attributor.attribute(endFile(901)).value(), generationA.value());
    QCOMPARE(
        attributor.attribute(propertyChange(MpvPropertyId::MediaTitle)).value(),
        generationA.value());

    QCOMPARE(attributor.attribute(startFile(902)).value(), generationB.value());
    QCOMPARE(
        attributor.attribute(propertyChange(MpvPropertyId::Position)).value(),
        generationA.value());

    QCOMPARE(attributor.attribute(fileLoaded()).value(), generationB.value());
    QCOMPARE(
        attributor.attribute(propertyChange(MpvPropertyId::Position)).value(),
        generationB.value());
}

void MediaGenerationTest::cancelledLoadDoesNotPoisonNextStartFile()
{
    MpvMediaGenerationAttributor attributor;
    attributor.noteLoadSubmission(
        player::ids::RequestId{2001},
        MediaGeneration{51});
    attributor.cancelLoadSubmission(player::ids::RequestId{2001});
    attributor.noteLoadSubmission(
        player::ids::RequestId{2002},
        MediaGeneration{52});

    QCOMPARE(attributor.attribute(startFile(601)).value(), quint64{52});
    QCOMPARE(attributor.attribute(fileLoaded()).value(), quint64{52});
}

void MediaGenerationTest::redirectEntriesKeepTheSameGeneration()
{
    MpvMediaGenerationAttributor attributor;
    const MediaGeneration generation{61};
    attributor.noteLoadSubmission(player::ids::RequestId{3001}, generation);

    QCOMPARE(attributor.attribute(startFile(701)).value(), generation.value());
    QCOMPARE(
        attributor.attribute(endFile(701, MpvEndFileReason::Redirect, 800, 2)).value(),
        generation.value());
    QCOMPARE(attributor.attribute(startFile(800)).value(), generation.value());
    QCOMPARE(attributor.attribute(fileLoaded()).value(), generation.value());
}

} // namespace player::playback::application

QTEST_GUILESS_MAIN(player::playback::application::MediaGenerationTest)
#include "media_generation_gate_test.moc"
