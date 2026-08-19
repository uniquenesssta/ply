#include <QDir>
#include <QFile>
#include <QString>
#include <QtTest>

namespace player::presentation::qml {
namespace {

QString readSource(const QString& relativePath)
{
    QFile file(QDir(QStringLiteral(PLAYER_SOURCE_DIR)).filePath(relativePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace

class TrackSelectionQmlTest final : public QObject
{
    Q_OBJECT

private slots:
    void popupUsesStableTrackIdsAndSnapshotSelection();
    void delayControlsRemainApplicationOwnedAndMpvFree();
    void utilityControlOwnsNoPlaybackTruth();
    void playerScreenPassesTrackSelectionDependencies();
};

void TrackSelectionQmlTest::popupUsesStableTrackIdsAndSnapshotSelection()
{
    const QString source = readSource(
        QStringLiteral("src/presentation/qml/features/tracks/TrackSelectionPopup.qml"));
    QVERIFY(!source.isEmpty());

    QVERIFY(source.contains(QStringLiteral("pragma ComponentBehavior: Bound")));
    QVERIFY(source.contains(QStringLiteral("selectAudioTrack(trackId)")));
    QVERIFY(source.contains(QStringLiteral("selectSubtitleTrack(trackId)")));
    QVERIFY(source.contains(QStringLiteral("disableSubtitles()")));
    QVERIFY(source.contains(QStringLiteral("required property var trackId")));
    QVERIFY(source.contains(QStringLiteral("required property bool selected")));
    QVERIFY(source.contains(QStringLiteral("subtitleTrackModel.selectedTrackId === 0")));
    QVERIFY(source.contains(QStringLiteral("AudioDelayControl")));
    QVERIFY(source.contains(QStringLiteral("audioDelayController")));
    QVERIFY(source.contains(QStringLiteral("SubtitleDelayControl")));
    QVERIFY(source.contains(QStringLiteral("subtitleDelayController")));
    QVERIFY(!source.contains(QStringLiteral("selectAudioTrack(index)")));
    QVERIFY(!source.contains(QStringLiteral("selectSubtitleTrack(index)")));
    QVERIFY(!source.contains(QStringLiteral("checkable: true")));
    QVERIFY(!source.contains(QStringLiteral("checked:")));
}

void TrackSelectionQmlTest::delayControlsRemainApplicationOwnedAndMpvFree()
{
    const QString audio = readSource(
        QStringLiteral("src/presentation/qml/features/tracks/AudioDelayControl.qml"));
    const QString subtitle = readSource(
        QStringLiteral("src/presentation/qml/features/tracks/SubtitleDelayControl.qml"));
    QVERIFY(!audio.isEmpty());
    QVERIFY(!subtitle.isEmpty());

    QVERIFY(audio.contains(QStringLiteral("objectName: \"audioDelaySlider\"")));
    QVERIFY(audio.contains(QStringLiteral("root.controller.setDelaySeconds")));
    QVERIFY(audio.contains(QStringLiteral("root.controller.resetDelay()")));
    QVERIFY(audio.contains(QStringLiteral("root.controller.pending")));
    QVERIFY(audio.contains(QStringLiteral("LayoutTokens.delayControlWidth")));
    QVERIFY(!audio.contains(QStringLiteral("subtitleDelay")));
    QVERIFY(!audio.contains(QStringLiteral("mpv_")));
    QVERIFY(!audio.contains(QStringLiteral("mpv_command")));

    QVERIFY(subtitle.contains(QStringLiteral("objectName: \"subtitleDelaySlider\"")));
    QVERIFY(!subtitle.contains(QStringLiteral("audioDelayController")));
    QVERIFY(!subtitle.contains(QStringLiteral("mpv_")));
    QVERIFY(!subtitle.contains(QStringLiteral("mpv_command")));
}

void TrackSelectionQmlTest::utilityControlOwnsNoPlaybackTruth()
{
    const QString source = readSource(
        QStringLiteral("src/presentation/qml/screens/player/osc/PlayerUtilityControls.qml"));
    QVERIFY(!source.isEmpty());

    QVERIFY(source.contains(QStringLiteral("TrackSelectionPopup")));
    QVERIFY(source.contains(QStringLiteral("trackSelectionController: root.trackSelectionController")));
    QVERIFY(source.contains(QStringLiteral("audioTrackModel: root.audioTrackModel")));
    QVERIFY(source.contains(QStringLiteral("subtitleTrackModel: root.subtitleTrackModel")));
    QVERIFY(source.contains(QStringLiteral("audioDelayController: root.audioDelayController")));
    QVERIFY(source.contains(QStringLiteral("subtitleDelayController: root.subtitleDelayController")));
    QVERIFY(!source.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!source.contains(QStringLiteral("mpv_")));
}

void TrackSelectionQmlTest::playerScreenPassesTrackSelectionDependencies()
{
    const QString source = readSource(
        QStringLiteral("src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString shell = readSource(
        QStringLiteral("src/presentation/qml/shell/MainWindow.qml"));
    QVERIFY(!source.isEmpty());
    QVERIFY(!shell.isEmpty());

    QVERIFY(source.contains(QStringLiteral("property var trackSelectionController: null")));
    QVERIFY(source.contains(QStringLiteral("property var audioDelayController: null")));
    QVERIFY(source.contains(QStringLiteral("property var subtitleDelayController: null")));
    QVERIFY(source.contains(QStringLiteral("trackSelectionController: root.trackSelectionController")));
    QVERIFY(source.contains(QStringLiteral("audioDelayController: root.audioDelayController")));
    QVERIFY(source.contains(QStringLiteral("subtitleDelayController: root.subtitleDelayController")));
    QVERIFY(source.contains(QStringLiteral("audioTrackModel: root.audioTrackModel")));
    QVERIFY(source.contains(QStringLiteral("subtitleTrackModel: root.subtitleTrackModel")));
    QVERIFY(source.contains(QStringLiteral("onTrackPopupOpenChanged: root.popupOpen = trackPopupOpen")));

    QVERIFY(shell.contains(QStringLiteral("property var audioDelayController: null")));
    QVERIFY(shell.contains(QStringLiteral("audioDelayController: window.audioDelayController")));
}

} // namespace player::presentation::qml

QTEST_GUILESS_MAIN(player::presentation::qml::TrackSelectionQmlTest)
#include "track_selection_qml_test.moc"
