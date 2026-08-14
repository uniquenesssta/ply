#include <QCoreApplication>
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

void verifyAbsent(const QString& source, const QStringList& forbidden)
{
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

} // namespace

class PlayerBottomRegionTest final : public QObject
{
    Q_OBJECT

private slots:
    void playerScreenComposesResponsiveOscHost();
    void oscLayoutOwnsSurfaceAndTwoLaneGrid();
    void controlRowKeepsFeatureSlotsSeparate();
    void oscSurfaceKeepsMaterialResponsibilityOnly();
};

void PlayerBottomRegionTest::playerScreenComposesResponsiveOscHost()
{
    const QString relativePath =
        QStringLiteral("src/presentation/qml/screens/player/PlayerScreen.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    QVERIFY(source.contains(QStringLiteral("property bool fullScreen: false")));
    QVERIFY(source.contains(QStringLiteral("readonly property bool oscCompact: root.fullScreen")));
    QVERIFY(source.contains(QStringLiteral("PlayerBottomRegion {")));
    QVERIFY(source.contains(QStringLiteral("PlayerOscLayout {")));
    QVERIFY(source.contains(QStringLiteral("compact: root.oscCompact")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscBottomCompact")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscBottom")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscHeightCompact")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscHeight")));

    verifyAbsent(
        source,
        {QStringLiteral("IconButton"),
         QStringLiteral("Slider {"),
         QStringLiteral("PlaybackSession"),
         QStringLiteral("libmpv"),
         QStringLiteral("mpv_")});
}

void PlayerBottomRegionTest::oscLayoutOwnsSurfaceAndTwoLaneGrid()
{
    const QString relativePath = QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerOscLayout.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    const QStringList slotContracts{
        QStringLiteral("property alias timelineContent: timelineHost.data"),
        QStringLiteral("property alias transportContent: controlRow.transportContent"),
        QStringLiteral("property alias volumeContent: controlRow.volumeContent"),
        QStringLiteral("property alias utilityContent: controlRow.utilityContent")};
    for (const QString& contract : slotContracts) {
        QVERIFY2(source.contains(contract), qPrintable(contract));
    }

    QVERIFY(source.contains(QStringLiteral("OscSurface {")));
    QVERIFY(source.contains(QStringLiteral("objectName: \"playerOscTimelineSlot\"")));
    QVERIFY(source.contains(QStringLiteral("OscControlRow {")));
    QVERIFY(source.contains(QStringLiteral("Math.min(")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscMaximumWidthCompact")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscMaximumWidth")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscInsetCompact")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscInset")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscPaddingTopCompact")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscPaddingTop")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscSectionGapCompact")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.oscSectionGap")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscTimelineLaneHeightCompact")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscTimelineLaneHeight")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscControlLaneHeight")));
    QVERIFY(source.contains(QStringLiteral("clip: false")));

    verifyAbsent(
        source,
        {QStringLiteral("IconButton"),
         QStringLiteral("Slider {"),
         QStringLiteral("PlaybackSession"),
         QStringLiteral("libmpv"),
         QStringLiteral("mpv_")});
}

void PlayerBottomRegionTest::controlRowKeepsFeatureSlotsSeparate()
{
    const QString relativePath = QStringLiteral(
        "src/presentation/qml/screens/player/osc/OscControlRow.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    const QStringList slotNames{
        QStringLiteral("playerOscTransportSlot"),
        QStringLiteral("playerOscVolumeSlot"),
        QStringLiteral("playerOscUtilitySlot")};
    for (const QString& slotName : slotNames) {
        QVERIFY2(source.contains(slotName), qPrintable(slotName));
    }

    QVERIFY(source.contains(QStringLiteral("property alias transportContent")));
    QVERIFY(source.contains(QStringLiteral("property alias volumeContent")));
    QVERIFY(source.contains(QStringLiteral("property alias utilityContent")));
    QVERIFY(source.contains(QStringLiteral("SpacingTokens.controlGroup")));
    QVERIFY(source.contains(QStringLiteral("readonly property bool contentConstrained")));
    QVERIFY(source.contains(QStringLiteral("id: leadingClip")));
    QVERIFY(source.contains(QStringLiteral("utilityHost.x")));
    QVERIFY(source.contains(QStringLiteral("width: Math.max(")));
    QVERIFY(source.contains(QStringLiteral("clip: true")));

    verifyAbsent(
        source,
        {QStringLiteral("IconButton"),
         QStringLiteral("Slider"),
         QStringLiteral("PlaybackSession"),
         QStringLiteral("libmpv"),
         QStringLiteral("mpv_")});
}

void PlayerBottomRegionTest::oscSurfaceKeepsMaterialResponsibilityOnly()
{
    const QString relativePath =
        QStringLiteral("src/presentation/qml/surfaces/OscSurface.qml");
    const QString source = readSource(relativePath);
    QVERIFY2(!source.isEmpty(), qPrintable(sourcePath(relativePath)));

    QVERIFY(source.contains(QStringLiteral("Panel {")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscMaximumWidthCompact")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscMaximumWidth")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscHeightCompact")));
    QVERIFY(source.contains(QStringLiteral("LayoutTokens.oscHeight")));
    QVERIFY(source.contains(QStringLiteral("RadiusTokens.surfaceOscCompact")));
    QVERIFY(source.contains(QStringLiteral("RadiusTokens.surfaceOsc")));
    QVERIFY(source.contains(QStringLiteral("MaterialTokens.oscCompactFillAlpha")));
    QVERIFY(source.contains(QStringLiteral("MaterialTokens.oscFillAlpha")));
    QVERIFY(source.contains(QStringLiteral("MaterialTokens.oscCompactBlur")));
    QVERIFY(source.contains(QStringLiteral("MaterialTokens.oscBlur")));
    QVERIFY(source.contains(QStringLiteral("contentPadding: 0")));
    QVERIFY(source.contains(QStringLiteral("ZOrderTokens.osc")));

    verifyAbsent(
        source,
        {QStringLiteral("Text {"),
         QStringLiteral("IconButton"),
         QStringLiteral("Slider"),
         QStringLiteral("PlaybackSession"),
         QStringLiteral("libmpv"),
         QStringLiteral("mpv_")});
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    player::presentation::qml::PlayerBottomRegionTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "player_bottom_region_test.moc"
