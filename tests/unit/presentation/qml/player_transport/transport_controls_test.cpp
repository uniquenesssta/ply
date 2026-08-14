#include <QFile>
#include <QString>
#include <QStringList>
#include <QtTest>

namespace player::presentation::qml {
namespace {

QString sourcePath()
{
    return QStringLiteral(
        PLAYER_SOURCE_DIR "/src/presentation/qml/features/player/transport/TransportControls.qml");
}

QString readSource()
{
    QFile file(sourcePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace

class TransportControlsTest final : public QObject
{
    Q_OBJECT

private slots:
    void usesCanonicalTransportControlsAndViewModelIntent();
    void remainsInsideFeaturePresentationBoundary();
};

void TransportControlsTest::usesCanonicalTransportControlsAndViewModelIntent()
{
    const QString source = readSource();
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath()));

    QVERIFY(source.contains(QStringLiteral("objectName: \"transportPreviousButton\"")));
    QVERIFY(source.contains(QStringLiteral("objectName: \"transportPlayPauseButton\"")));
    QVERIFY(source.contains(QStringLiteral("objectName: \"transportNextButton\"")));
    QVERIFY(source.contains(QStringLiteral("iconId: \"previous\"")));
    QVERIFY(source.contains(QStringLiteral("iconId: \"play\"")));
    QVERIFY(source.contains(QStringLiteral("iconId: \"next\"")));
    QVERIFY(source.contains(QStringLiteral("emphasis: IconButton.Primary")));

    QVERIFY(source.contains(QStringLiteral("root.viewModel.canPrevious")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.canPlay")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.canPause")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.canNext")));
    QVERIFY(source.contains(QStringLiteral("root.viewModel.isPlaying")));
    QVERIFY(source.contains(QStringLiteral("qsTr(\"Pause\")")));
    QVERIFY(source.contains(QStringLiteral("qsTr(\"Play\")")));
    QVERIFY(source.contains(QStringLiteral("requestTogglePlayPause()")));
}

void TransportControlsTest::remainsInsideFeaturePresentationBoundary()
{
    const QString source = readSource();
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath()));

    QVERIFY(source.contains(QStringLiteral("import Player.Presentation.Theme")));
    QVERIFY(source.contains(QStringLiteral("import Player.Presentation.Controls")));

    const QStringList forbidden{
        QStringLiteral("Player.Presentation.Primitives"),
        QStringLiteral("Player.Presentation.Surfaces"),
        QStringLiteral("PlaybackSession"),
        QStringLiteral("PlaybackCommandBus"),
        QStringLiteral("TransportCommand"),
        QStringLiteral("libmpv"),
        QStringLiteral("mpv_")};
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

} // namespace player::presentation::qml

QTEST_GUILESS_MAIN(player::presentation::qml::TransportControlsTest)
#include "transport_controls_test.moc"
