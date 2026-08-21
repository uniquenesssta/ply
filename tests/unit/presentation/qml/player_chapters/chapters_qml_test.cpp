#include <QAbstractListModel>
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <QtTest>

#include <memory>

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

QString componentDiagnostics(const QQmlComponent& component)
{
    QStringList diagnostics;
    for (const QQmlError& error : component.errors()) {
        diagnostics.append(error.toString());
    }
    return diagnostics.join(QLatin1Char('\n'));
}

bool waitForComponentResolution(QQmlComponent& component)
{
    if (component.status() != QQmlComponent::Loading) {
        return true;
    }

    QSignalSpy statusSpy(&component, &QQmlComponent::statusChanged);
    return statusSpy.wait(5000);
}

class ChapterRowsModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count CONSTANT)

public:
    enum Role {
        IndexRole = Qt::UserRole + 1,
        TitleRole,
        TimeTextRole,
    };

    explicit ChapterRowsModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex{}) const override
    {
        return parent.isValid() ? 0 : 1;
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() != 0) {
            return {};
        }

        switch (role) {
        case IndexRole:
            return 0;
        case TitleRole:
            return QStringLiteral("Intro");
        case TimeTextRole:
            return QStringLiteral("00:00:00");
        default:
            return {};
        }
    }

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override
    {
        return {
            {IndexRole, QByteArrayLiteral("index")},
            {TitleRole, QByteArrayLiteral("title")},
            {TimeTextRole, QByteArrayLiteral("timeText")},
        };
    }

    [[nodiscard]] int count() const noexcept
    {
        return 1;
    }
};

} // namespace

class ChaptersQmlTest final : public QObject
{
    Q_OBJECT

private slots:
    void chapterRowsUseReadonlyModelAndExplicitSeekIntent();
    void chapterDelegateReceivesRequiredModelRolesAtRuntime();
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
    QVERIFY(row.contains(QStringLiteral("IconButton {")));
    QVERIFY(!row.contains(QStringLiteral("ButtonBase {")));
    QVERIFY(content.contains(QStringLiteral("model: root.chapterModel")));
    QVERIFY(content.contains(QStringLiteral("required property var index")));
    QVERIFY(!content.contains(QStringLiteral("required property string title")));
    QVERIFY(!content.contains(QStringLiteral("required property string timeText")));
    QVERIFY(row.contains(QStringLiteral("required property string title")));
    QVERIFY(row.contains(QStringLiteral("required property string timeText")));
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

void ChaptersQmlTest::chapterDelegateReceivesRequiredModelRolesAtRuntime()
{
    QQmlEngine engine;
    QStringList qmlWarnings;
    QObject::connect(
        &engine,
        &QQmlEngine::warnings,
        &engine,
        [&qmlWarnings](const QList<QQmlError>& warnings) {
            for (const QQmlError& warning : warnings) {
                qmlWarnings.append(warning.toString());
            }
        });

    const QString path = QStringLiteral(
        PLAYER_SOURCE_DIR "/src/presentation/qml/features/chapters/ChapterContent.qml");
    QQmlComponent component(&engine, QUrl::fromLocalFile(path));
    const bool resolved = waitForComponentResolution(component);
    const QString loadDiagnostics = componentDiagnostics(component);
    QVERIFY2(resolved, qPrintable(loadDiagnostics));
    QVERIFY2(component.isReady(), qPrintable(loadDiagnostics));

    ChapterRowsModel model;
    QVariantMap initialProperties;
    initialProperties.insert(QStringLiteral("width"), 480);
    initialProperties.insert(QStringLiteral("height"), 320);
    initialProperties.insert(
        QStringLiteral("chapterModel"),
        QVariant::fromValue(static_cast<QObject*>(&model)));

    std::unique_ptr<QObject> content(
        component.createWithInitialProperties(initialProperties));
    const QString createDiagnostics = componentDiagnostics(component);
    QVERIFY2(content != nullptr, qPrintable(createDiagnostics));
    QCoreApplication::processEvents();

    QTRY_COMPARE_WITH_TIMEOUT(
        content->findChildren<QObject*>(QStringLiteral("chapterRow")).size(),
        1,
        1000);
    QObject* row = content->findChild<QObject*>(QStringLiteral("chapterRow"));
    QVERIFY(row != nullptr);
    QCOMPARE(row->property("chapterIndex").toLongLong(), qint64{0});
    QCOMPARE(row->property("title").toString(), QStringLiteral("Intro"));
    QCOMPARE(row->property("timeText").toString(), QStringLiteral("00:00:00"));
    QVERIFY2(qmlWarnings.isEmpty(), qPrintable(qmlWarnings.join(QLatin1Char('\n'))));
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
    const QString controls = readSource(QStringLiteral(
        "src/presentation/qml/features/chapters/ChapterControls.qml"));
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
    const QString chapterModel = readSource(QStringLiteral(
        "src/chapters/presentation/chapter_model.cpp"));
    const QString chapterState = readSource(QStringLiteral(
        "src/playback/domain/state/playback_chapter_state.cpp"));
    QVERIFY(timeline.contains(QStringLiteral("property var chapterModel: null")));
    QVERIFY(timeline.startsWith(QStringLiteral("pragma ComponentBehavior: Bound")));
    QVERIFY(timeline.contains(QStringLiteral("model: root.chapterModel")));
    QVERIFY(timeline.contains(QStringLiteral(
        "required property double normalizedTime")));
    QVERIFY(timeline.contains(QStringLiteral("root.viewModel.durationSeconds")));
    QVERIFY(!timeline.contains(QStringLiteral("required property double time")));
    QVERIFY(!timeline.contains(QStringLiteral("Math.min(")));
    QVERIFY(!timeline.contains(QStringLiteral(
        "time / root.viewModel.durationSeconds")));
    QVERIFY(!timeline.contains(QStringLiteral("timelineSlider.value = time")));
    QVERIFY(!timeline.contains(QStringLiteral(
        "timelineSlider.value = normalizedTime")));
    QVERIFY(chapterModel.contains(QStringLiteral("NormalizedTimeRole")));
    QVERIFY(chapterModel.contains(QStringLiteral("validatedForDuration")));
    QVERIFY(chapterState.contains(QStringLiteral(
        "chapter.startSeconds > *durationSeconds")));
    QVERIFY(screen.contains(QStringLiteral(
        "navigationViewModel: root.chapterNavigationViewModel")));
    QVERIFY(utility.contains(QStringLiteral("ChapterControls {")));
    QVERIFY(controls.contains(QStringLiteral("IconButton {")));
    QVERIFY(!controls.contains(QStringLiteral("ButtonBase {")));
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

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::ChaptersQmlTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "chapters_qml_test.moc"
