#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
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

void verifyAbsent(const QString& source, const QStringList& forbidden)
{
    for (const QString& token : forbidden) {
        QVERIFY2(!source.contains(token), qPrintable(token));
    }
}

} // namespace

class ChaptersQmlTest final : public QObject
{
    Q_OBJECT

private slots:
    void chapterRowsUseReadonlyModelAndExplicitSeekIntent();
    void chapterCurrentPendingAndNonSeekableStatesRemainSeparate();
    void inspectorReusesCanonicalShellAndChapterGeometry();
    void screenAndBootstrapPassChapterDependencies();
    void qmlRegistrationIncludesChapterFeature();
};

void ChaptersQmlTest::chapterRowsUseReadonlyModelAndExplicitSeekIntent()
{
    const QString content = readSource(QStringLiteral(
        "src/presentation/qml/features/chapters/ChapterContent.qml"));
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/chapters/ChapterRow.qml"));

    QVERIFY(!content.isEmpty());
    QVERIFY(!row.isEmpty());
    QVERIFY(!content.contains(QStringLiteral("Player.Presentation.Primitives")));
    QVERIFY(content.contains(QStringLiteral("model: root.chapterModel")));
    QVERIFY(content.contains(QStringLiteral("required property var index")));
    QVERIFY(content.contains(QStringLiteral("required property string timeText")));
    QVERIFY(content.contains(QStringLiteral(
        "root.navigationViewModel.requestChapterSeek(requestedIndex)")));
    QVERIFY(row.contains(QStringLiteral("signal seekRequested(var chapterIndex)")));
    QVERIFY(row.contains(QStringLiteral("onClicked: root.seekRequested(root.chapterIndex)")));

    verifyAbsent(content + row,
                 {QStringLiteral("timelineSlider"),
                  QStringLiteral("displayedNormalized"),
                  QStringLiteral("beginScrub"),
                  QStringLiteral("commitScrub"),
                  QStringLiteral("PlaybackSession"),
                  QStringLiteral("PlaybackCommandBus"),
                  QStringLiteral("mpv_")});
}

void ChaptersQmlTest::chapterCurrentPendingAndNonSeekableStatesRemainSeparate()
{
    const QString content = readSource(QStringLiteral(
        "src/presentation/qml/features/chapters/ChapterContent.qml"));
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/chapters/ChapterRow.qml"));

    QVERIFY(content.contains(QStringLiteral(
        "root.navigationViewModel.currentChapterIndex === chapterIndex")));
    QVERIFY(content.contains(QStringLiteral(
        "root.navigationViewModel.pendingChapterIndex === chapterIndex")));
    QVERIFY(content.contains(QStringLiteral(
        "root.navigationViewModel.canSeek")));
    QVERIFY(row.contains(QStringLiteral("property bool current: false")));
    QVERIFY(row.contains(QStringLiteral("property bool pending: false")));
    QVERIFY(row.contains(QStringLiteral("property bool seekEnabled: false")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.controlPendingTarget")));
    QVERIFY(row.contains(QStringLiteral("ColorTokens.surfaceInspectorSelection")));
}

void ChaptersQmlTest::inspectorReusesCanonicalShellAndChapterGeometry()
{
    const QString inspector = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/ChaptersInspector.qml"));
    const QString content = readSource(QStringLiteral(
        "src/presentation/qml/features/chapters/ChapterContent.qml"));
    const QString row = readSource(QStringLiteral(
        "src/presentation/qml/features/chapters/ChapterRow.qml"));
    const QString footer = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/drawers/ChapterFooter.qml"));

    QVERIFY(inspector.contains(QStringLiteral("PlayerInspectorShell {")));
    QVERIFY(inspector.contains(QStringLiteral("ChapterModeIndicator {")));
    QVERIFY(inspector.contains(QStringLiteral("ChapterContent {")));
    QVERIFY(inspector.contains(QStringLiteral("ChapterFooter {")));
    QVERIFY(content.contains(QStringLiteral("ListView {")));
    QVERIFY(content.contains(QStringLiteral("reuseItems: true")));
    QVERIFY(content.contains(QStringLiteral("spacing: SpacingTokens.listGap")));
    QVERIFY(row.contains(QStringLiteral("height: LayoutTokens.listRowHeight")));
    QVERIFY(row.contains(QStringLiteral("radius: RadiusTokens.listRow")));
    QVERIFY(row.contains(QStringLiteral("TypographyTokens.microStrong")));
    QVERIFY(row.contains(QStringLiteral("TypographyTokens.timecodeExtraSmall")));
    QVERIFY(footer.contains(QStringLiteral("RadiusTokens.inspectorFooter")));
    QVERIFY(footer.contains(QStringLiteral("requestPreviousChapter()")));
    QVERIFY(footer.contains(QStringLiteral("requestNextChapter()")));
}

void ChaptersQmlTest::screenAndBootstrapPassChapterDependencies()
{
    const QString window = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString utility = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/osc/PlayerUtilityControls.qml"));
    const QString bootstrap = readSource(QStringLiteral(
        "src/app/bootstrap/application_bootstrap.cpp"));
    const QString container = readSource(QStringLiteral(
        "src/app/composition/application_container.cpp"));
    const QString composition = readSource(QStringLiteral(
        "src/app/composition/playback_composition.cpp"));

    QVERIFY(window.contains(QStringLiteral("property var chapterModel: null")));
    QVERIFY(window.contains(QStringLiteral(
        "property var chapterNavigationViewModel: null")));
    QVERIFY(screen.contains(QStringLiteral("property bool chapterDrawerOpen: false")));
    QVERIFY(screen.contains(QStringLiteral("|| root.chapterDrawerOpen")));
    QVERIFY(screen.contains(QStringLiteral("ChaptersInspector {")));
    QVERIFY(screen.contains(QStringLiteral("chapterModel: root.chapterModel")));
    const QString timeline = readSource(QStringLiteral(
        "src/presentation/qml/features/player/timeline/TimelineControls.qml"));
    QVERIFY(timeline.contains(QStringLiteral("property var chapterModel: null")));
    QVERIFY(timeline.contains(QStringLiteral("model: root.chapterModel")));
    QVERIFY(timeline.contains(QStringLiteral("required property double time")));
    QVERIFY(timeline.contains(QStringLiteral("root.viewModel.durationSeconds")));
    QVERIFY(!timeline.contains(QStringLiteral("timelineSlider.value = time")));
    QVERIFY(screen.contains(QStringLiteral(
        "navigationViewModel: root.chapterNavigationViewModel")));
    QVERIFY(utility.contains(QStringLiteral("ChapterControls {")));
    QVERIFY(utility.contains(QStringLiteral("signal toggleChaptersRequested()")));
    const qsizetype playlistPosition = utility.indexOf(QStringLiteral("PlaylistControls {"));
    const qsizetype chapterPosition = utility.indexOf(QStringLiteral("ChapterControls {"));
    const qsizetype fullscreenPosition = utility.indexOf(QStringLiteral("FullscreenControls {"));
    QVERIFY(playlistPosition >= 0);
    QVERIFY(chapterPosition > playlistPosition);
    QVERIFY(fullscreenPosition > chapterPosition);
    QVERIFY(screen.contains(QStringLiteral("root.playlistDrawerOpen = false")));
    QVERIFY(screen.contains(QStringLiteral("root.chapterDrawerOpen = false")));
    QVERIFY(bootstrap.contains(QStringLiteral("QStringLiteral(\"chapterModel\")")));
    QVERIFY(bootstrap.contains(QStringLiteral(
        "QStringLiteral(\"chapterNavigationViewModel\")")));
    QVERIFY(container.contains(QStringLiteral(
        "ChapterNavigationViewModel::seekRequested")));
    QVERIFY(container.contains(QStringLiteral("SeekMode::Absolute")));
    QVERIFY(composition.contains(QStringLiteral(
        "PlaybackComposition::submitSeek(\n    const player::playback::domain::SeekCommand& seek)")));
}

void ChaptersQmlTest::qmlRegistrationIncludesChapterFeature()
{
    const QString cmake = readSource(QStringLiteral("src/presentation/CMakeLists.txt"));
    QVERIFY(!cmake.isEmpty());

    const QStringList requiredPaths{
        QStringLiteral("qml/screens/player/drawers/ChaptersInspector.qml"),
        QStringLiteral("qml/screens/player/drawers/ChapterModeIndicator.qml"),
        QStringLiteral("qml/screens/player/drawers/ChapterFooter.qml"),
        QStringLiteral("qml/features/chapters/ChapterControls.qml"),
        QStringLiteral("qml/features/chapters/ChapterContent.qml"),
        QStringLiteral("qml/features/chapters/ChapterMarkerGlyph.qml"),
        QStringLiteral("qml/features/chapters/ChapterRow.qml"),
    };
    for (const QString& path : requiredPaths) {
        QVERIFY2(cmake.contains(path), qPrintable(path));
    }
}

} // namespace player::presentation::qml

QTEST_GUILESS_MAIN(player::presentation::qml::ChaptersQmlTest)
#include "chapters_qml_test.moc"
