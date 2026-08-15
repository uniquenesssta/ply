#include <QFile>
#include <QString>
#include <QStringList>
#include <QtTest>

namespace player::presentation::qml {
namespace {

QString sourcePath(const QString& relativePath)
{
    return QStringLiteral(PLAYER_SOURCE_DIR) + QLatin1Char('/') + relativePath;
}

QString readSource(const QString& relativePath)
{
    QFile file(sourcePath(relativePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace

class TimelineControlsTest final : public QObject
{
    Q_OBJECT

private slots:
    void timelineUsesCanonicalGeometryAndGenericSlider();
    void timelineOwnsInteractionIntentWithoutBackendAccess();
    void screenAndBootstrapInjectTimelineStateExplicitly();
    void compositionRoutesAbsoluteAndRelativeSeekThroughSharedBus();
};

void TimelineControlsTest::timelineUsesCanonicalGeometryAndGenericSlider()
{
    const QString relativePath = QStringLiteral(
        "src/presentation/qml/features/player/timeline/TimelineControls.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    QVERIFY(source.contains(QStringLiteral("Slider {")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.timelineHitHeight")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.sliderTrackInset")));
    QVERIFY(source.contains(QStringLiteral("TypographyTokens.timecodeMediumPrimary")));
    QVERIFY(source.contains(QStringLiteral("TypographyTokens.timecodeMediumSecondary")));
    QVERIFY(source.contains(QStringLiteral("TypographyTokens.timecodeSmallPrimary")));
    QVERIFY(source.contains(QStringLiteral("TypographyTokens.timecodeSmallSecondary")));
    QVERIFY(source.contains(QStringLiteral("objectName: \"timelineCurrentTime\"")));
    QVERIFY(source.contains(QStringLiteral("objectName: \"timelineDurationTime\"")));
    QVERIFY(source.contains(QStringLiteral("showValue: false")));
    QVERIFY(source.contains(QStringLiteral("keyboardEnabled: false")));
    QVERIFY(source.contains(QStringLiteral("wheelEnabled: false")));
    QVERIFY(source.contains(QStringLiteral("WheelHandler {")));
    QVERIFY(source.contains(QStringLiteral("\"Disabled\"")));
    QVERIFY(source.contains(QStringLiteral("\"Scrubbing\"")));
    QVERIFY(source.contains(QStringLiteral("\"CommitPending\"")));
    QVERIFY(source.contains(QStringLiteral("\"Hovering\"")));
    QVERIFY(source.contains(QStringLiteral("\"Idle\"")));

    const QString sliderSource = readSource(QStringLiteral(
        "src/presentation/qml/controls/sliders/Slider.qml"));
    QVERIFY2(!sliderSource.isEmpty(), qPrintable(sourcePath(QStringLiteral(
        "src/presentation/qml/controls/sliders/Slider.qml"))));
    QVERIFY(sliderSource.contains(QStringLiteral(
        "property bool keyboardEnabled: true")));
    QVERIFY(sliderSource.contains(QStringLiteral(
        "height: LayoutTokens.sliderTrackHeight")));

    const QString oscSource = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerOscLayout.qml"));
    QVERIFY(oscSource.contains(QStringLiteral("clip: false")));
}

void TimelineControlsTest::timelineOwnsInteractionIntentWithoutBackendAccess()
{
    const QString relativePath = QStringLiteral(
        "src/presentation/qml/features/player/timeline/TimelineControls.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    QVERIFY(source.contains(QStringLiteral("import Player.Presentation.Theme")));
    QVERIFY(source.contains(QStringLiteral("import Player.Presentation.Controls")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.beginScrub")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.updateScrub")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.commitScrub")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.cancelScrub")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.requestRelativeSeek")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.relativeSeekStepSeconds")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.displayedNormalized")));
    QVERIFY(source.contains(QStringLiteral("property bool windowActive: true")));
    QVERIFY(source.contains(QStringLiteral("onWindowActiveChanged")));
    QVERIFY(!source.contains(QStringLiteral("MouseArea")));

    const QStringList forbidden{
        QStringLiteral("Player.Presentation.Primitives"),
        QStringLiteral("Player.Presentation.Surfaces"),
        QStringLiteral("PlaybackSession"),
        QStringLiteral("PlaybackCommandBus"),
        QStringLiteral("SeekCommand"),
        QStringLiteral("libmpv"),
        QStringLiteral("mpv_")};
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

void TimelineControlsTest::screenAndBootstrapInjectTimelineStateExplicitly()
{
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString window = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    const QString bootstrap = readSource(QStringLiteral(
        "src/app/bootstrap/application_bootstrap.cpp"));

    QVERIFY(screen.contains(QStringLiteral("property var timelineViewModel: null")));
    QVERIFY(screen.contains(QStringLiteral("property bool windowActive: true")));
    QVERIFY(screen.contains(QStringLiteral("timelineContent: [")));
    QVERIFY(screen.contains(QStringLiteral("TimelineControls {")));
    QVERIFY(screen.contains(QStringLiteral("windowActive: root.windowActive")));
    QVERIFY(screen.contains(QStringLiteral("viewModel: root.timelineViewModel")));

    QVERIFY(window.contains(QStringLiteral("property var timelineViewModel: null")));
    QVERIFY(window.contains(QStringLiteral("timelineViewModel: window.timelineViewModel")));
    QVERIFY(window.contains(QStringLiteral("windowActive: window.active")));

    QVERIFY(bootstrap.contains(QStringLiteral("\"timelineViewModel\"")));
    QVERIFY(bootstrap.contains(QStringLiteral("playbackComposition.timelineViewModel()")));
}

void TimelineControlsTest::compositionRoutesAbsoluteAndRelativeSeekThroughSharedBus()
{
    const QString composition = readSource(QStringLiteral(
        "src/app/composition/playback_composition.cpp"));
    QVERIFY(!composition.isEmpty());

    QVERIFY(composition.contains(QStringLiteral(
        "&player::presentation::PlayerTimelineViewModel::seekRequested")));
    QVERIFY(composition.contains(QStringLiteral(
        "&player::presentation::PlayerTimelineViewModel::relativeSeekRequested")));
    QVERIFY(composition.contains(QStringLiteral(
        "submitSeek(absoluteSeconds, SeekMode::Absolute)")));
    QVERIFY(composition.contains(QStringLiteral(
        "submitSeek(deltaSeconds, SeekMode::Relative)")));
    QVERIFY(composition.contains(QStringLiteral("requestIdGenerator_->next()")));
    QVERIFY(composition.contains(QStringLiteral("SeekCommand{seconds, mode}")));
    QVERIFY(composition.contains(QStringLiteral("bus->submit(command")));
    QVERIFY(composition.contains(QStringLiteral("rejectPendingSeek()")));
}

} // namespace player::presentation::qml

QTEST_GUILESS_MAIN(player::presentation::qml::TimelineControlsTest)
#include "timeline_controls_test.moc"
