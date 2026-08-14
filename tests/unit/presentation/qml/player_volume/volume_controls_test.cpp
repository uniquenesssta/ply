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

class VolumeControlsTest final : public QObject
{
    Q_OBJECT

private slots:
    void volumeUsesCanonicalIconTrackAndGenericSlider();
    void compactModeKeepsIconAndHidesSlider();
    void volumeOwnsIntentMappingWithoutBackendAccess();
    void screenAndBootstrapInjectVolumeViewModelExplicitly();
    void compositionSubmitsVolumeAndMuteThroughSharedBus();
};

void VolumeControlsTest::volumeUsesCanonicalIconTrackAndGenericSlider()
{
    const QString relativePath = QStringLiteral(
        "src/presentation/qml/features/player/volume/VolumeControls.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    QVERIFY(source.contains(QStringLiteral("IconButton {")));
    QVERIFY(source.contains(QStringLiteral("iconId: \"volume\"")));
    QVERIFY(source.contains(QStringLiteral("Slider {")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.volumeTrackWidth")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.sliderTrackInset * 2")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.controlHitMinimum")));
    QVERIFY(source.contains(QStringLiteral("showValue: false")));
    QVERIFY(source.contains(QStringLiteral("wheelEnabled: true")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.requestVolumeNormalized")));
}

void VolumeControlsTest::compactModeKeepsIconAndHidesSlider()
{
    const QString source = readSource(QStringLiteral(
        "src/presentation/qml/features/player/volume/VolumeControls.qml"));
    QVERIFY(!source.isEmpty());

    QVERIFY(source.contains(QStringLiteral("property bool compact: false")));
    QVERIFY(source.contains(QStringLiteral("visible: !root.compact")));
    QVERIFY(source.contains(QStringLiteral("objectName: \"volumeMuteButton\"")));
    QVERIFY(source.contains(QStringLiteral("objectName: \"volumeSlider\"")));
}

void VolumeControlsTest::volumeOwnsIntentMappingWithoutBackendAccess()
{
    const QString source = readSource(QStringLiteral(
        "src/presentation/qml/features/player/volume/VolumeControls.qml"));
    QVERIFY(!source.isEmpty());

    QVERIFY(source.contains(QStringLiteral("import Player.Presentation.Theme")));
    QVERIFY(source.contains(QStringLiteral("import Player.Presentation.Controls")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.requestToggleMuted()")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.normalizedVolume")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.mutePending")));
    QVERIFY(!source.contains(QStringLiteral("MouseArea")));
    QVERIFY(!source.contains(QStringLiteral("iconId: \"mute\"")));

    const QStringList forbidden{
        QStringLiteral("Player.Presentation.Primitives"),
        QStringLiteral("Player.Presentation.Surfaces"),
        QStringLiteral("PlaybackSession"),
        QStringLiteral("PlaybackCommandBus"),
        QStringLiteral("SetVolumeCommand"),
        QStringLiteral("SetMutedCommand"),
        QStringLiteral("libmpv"),
        QStringLiteral("mpv_")};
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

void VolumeControlsTest::screenAndBootstrapInjectVolumeViewModelExplicitly()
{
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString window = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    const QString bootstrap = readSource(QStringLiteral(
        "src/app/bootstrap/application_bootstrap.cpp"));

    QVERIFY(screen.contains(QStringLiteral("property var volumeViewModel: null")));
    QVERIFY(screen.contains(QStringLiteral("volumeContent: [")));
    QVERIFY(screen.contains(QStringLiteral("VolumeControls {")));
    QVERIFY(screen.contains(QStringLiteral("compact: root.oscCompact")));
    QVERIFY(screen.contains(QStringLiteral("viewModel: root.volumeViewModel")));

    QVERIFY(window.contains(QStringLiteral("property var volumeViewModel: null")));
    QVERIFY(window.contains(QStringLiteral("volumeViewModel: window.volumeViewModel")));

    QVERIFY(bootstrap.contains(QStringLiteral("\"volumeViewModel\"")));
    QVERIFY(bootstrap.contains(QStringLiteral("playbackComposition.volumeViewModel()")));
}

void VolumeControlsTest::compositionSubmitsVolumeAndMuteThroughSharedBus()
{
    const QString composition = readSource(QStringLiteral(
        "src/app/composition/playback_composition.cpp"));
    QVERIFY(!composition.isEmpty());

    QVERIFY(composition.contains(QStringLiteral(
        "&player::presentation::PlayerVolumeViewModel::volumeRequested")));
    QVERIFY(composition.contains(QStringLiteral(
        "&player::presentation::PlayerVolumeViewModel::mutedRequested")));
    QVERIFY(composition.contains(QStringLiteral("submitVolume(percent)")));
    QVERIFY(composition.contains(QStringLiteral("submitMuted(muted)")));
    QVERIFY(composition.contains(QStringLiteral("requestIdGenerator_->next()")));
    QVERIFY(composition.contains(QStringLiteral("SetVolumeCommand{percent}")));
    QVERIFY(composition.contains(QStringLiteral("SetMutedCommand{muted}")));
    QVERIFY(composition.contains(QStringLiteral("bus->submit(command")));
    QVERIFY(composition.contains(QStringLiteral("rejectPendingVolume()")));
    QVERIFY(composition.contains(QStringLiteral("rejectPendingMute()")));
}

} // namespace player::presentation::qml

QTEST_GUILESS_MAIN(player::presentation::qml::VolumeControlsTest)
#include "volume_controls_test.moc"
