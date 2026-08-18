#include "tracks/application/track_selection_controller.h"

#include <QtTest>

#include <optional>
#include <vector>

namespace player::tracks::application {
namespace {

using player::playback::domain::TrackSelectionCommand;
using player::playback::domain::TrackSelectionKind;

} // namespace

class TrackSelectionControllerTest final : public QObject
{
    Q_OBJECT

private slots:
    void submitsStableAudioAndSubtitleIds();
    void subtitleOffIsExplicitCommandState();
    void rejectsInvalidIdsBeforeSubmission();
    void propagatesImmediateSubmissionFailure();
};

void TrackSelectionControllerTest::submitsStableAudioAndSubtitleIds()
{
    std::vector<TrackSelectionCommand> submitted;
    TrackSelectionController controller(
        [&submitted](const TrackSelectionCommand& command) {
            submitted.push_back(command);
            return true;
        });

    QVERIFY(controller.selectAudioTrack(3));
    QVERIFY(controller.selectSubtitleTrack(7));
    QCOMPARE(submitted.size(), std::size_t{2});
    QVERIFY(submitted.at(0).kind == TrackSelectionKind::Audio);
    QVERIFY(submitted.at(0).trackId.has_value());
    QCOMPARE(*submitted.at(0).trackId, qint64{3});
    QVERIFY(submitted.at(1).kind == TrackSelectionKind::Subtitle);
    QVERIFY(submitted.at(1).trackId.has_value());
    QCOMPARE(*submitted.at(1).trackId, qint64{7});
}

void TrackSelectionControllerTest::subtitleOffIsExplicitCommandState()
{
    std::optional<TrackSelectionCommand> submitted;
    TrackSelectionController controller(
        [&submitted](const TrackSelectionCommand& command) {
            submitted = command;
            return true;
        });

    QVERIFY(controller.disableSubtitles());
    QVERIFY(submitted.has_value());
    QVERIFY(submitted->kind == TrackSelectionKind::Subtitle);
    QVERIFY(!submitted->trackId.has_value());
}

void TrackSelectionControllerTest::rejectsInvalidIdsBeforeSubmission()
{
    int submissionCount = 0;
    TrackSelectionController controller(
        [&submissionCount](const TrackSelectionCommand&) {
            ++submissionCount;
            return true;
        });

    QVERIFY(!controller.selectAudioTrack(0));
    QVERIFY(!controller.selectAudioTrack(-1));
    QVERIFY(!controller.selectSubtitleTrack(0));
    QVERIFY(!controller.selectSubtitleTrack(-1));
    QCOMPARE(submissionCount, 0);
}

void TrackSelectionControllerTest::propagatesImmediateSubmissionFailure()
{
    TrackSelectionController controller(
        [](const TrackSelectionCommand&) {
            return false;
        });

    QVERIFY(!controller.selectAudioTrack(2));
    QVERIFY(!controller.selectSubtitleTrack(7));
    QVERIFY(!controller.disableSubtitles());
}

} // namespace player::tracks::application

QTEST_GUILESS_MAIN(player::tracks::application::TrackSelectionControllerTest)
#include "track_selection_controller_test.moc"
