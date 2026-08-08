#include "app/bootstrap/runtime_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>

namespace player::app {
namespace {

QString cleanPath(const QString& path)
{
    return path.isEmpty() ? QString{} : QDir::cleanPath(QDir::fromNativeSeparators(path));
}

QString childPath(const QString& parent, const QString& child)
{
    return cleanPath(QDir(parent).filePath(child));
}

bool writeTextFile(const QString& path, const QByteArray& content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    return file.write(content) == content.size();
}

} // namespace

class RuntimePathsTest final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void installedModeUsesStandardLocations();
    void currentProcessExecutableUsesLoadedModulePath();
    void executableFilePathUsesDevelopmentMarkerBeforeGuiApplication();
    void developmentMarkerWritesLogToProjectRoot();
    void developmentMarkerOverridesPortableMode();
    void invalidDevelopmentMarkerFallsBackToPortableMode();
    void portableModeStaysBesideExecutable();
    void portableMarkerControlsAutomaticModeDetection();
};

void RuntimePathsTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName(QStringLiteral("ModularPlayer"));
    QCoreApplication::setApplicationName(QStringLiteral("Player"));
}

void RuntimePathsTest::cleanupTestCase()
{
    QStandardPaths::setTestModeEnabled(false);
}

void RuntimePathsTest::installedModeUsesStandardLocations()
{
    QTemporaryDir executableDirectory;
    QVERIFY(executableDirectory.isValid());

    const RuntimePaths paths = RuntimePaths::resolve(
        RuntimePaths::Mode::Installed,
        executableDirectory.path());

    const QString expectedConfig = cleanPath(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString expectedData = cleanPath(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    const QString pictures = cleanPath(
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    const QString expectedScreenshots = pictures.isEmpty()
        ? childPath(expectedData, QStringLiteral("screenshots"))
        : childPath(pictures, QCoreApplication::applicationName());

    QCOMPARE(paths.mode(), RuntimePaths::Mode::Installed);
    QCOMPARE(paths.executableDirectory(), cleanPath(executableDirectory.path()));
    QCOMPARE(paths.configDirectory(), expectedConfig);
    QCOMPARE(paths.dataDirectory(), expectedData);
    QCOMPARE(paths.logDirectory(), childPath(expectedData, QStringLiteral("logs")));
    QCOMPARE(paths.screenshotDirectory(), expectedScreenshots);
}

void RuntimePathsTest::currentProcessExecutableUsesLoadedModulePath()
{
    const RuntimePaths paths = RuntimePaths::fromCurrentProcessExecutable();

    QCOMPARE(
        paths.executableDirectory(),
        cleanPath(QCoreApplication::applicationDirPath()));
}

void RuntimePathsTest::executableFilePathUsesDevelopmentMarkerBeforeGuiApplication()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString projectDirectory = childPath(
        temporaryDirectory.path(),
        QStringLiteral("project root with spaces"));
    const QString executableDirectory = childPath(
        projectDirectory,
        QStringLiteral("build/windows-msvc-debug"));
    QVERIFY(QDir().mkpath(executableDirectory));

    const QString markerPath = childPath(
        executableDirectory,
        QStringLiteral(".player-development-root"));
    QVERIFY(writeTextFile(markerPath, QByteArray("../..\n")));

    const QString executableFilePath = childPath(
        executableDirectory,
        QStringLiteral("Player.exe"));
    const RuntimePaths paths = RuntimePaths::fromExecutableFilePath(executableFilePath);

    QCOMPARE(paths.mode(), RuntimePaths::Mode::Installed);
    QCOMPARE(paths.executableDirectory(), executableDirectory);
    QCOMPARE(paths.logDirectory(), projectDirectory);
}

void RuntimePathsTest::developmentMarkerWritesLogToProjectRoot()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString projectDirectory = childPath(
        temporaryDirectory.path(),
        QStringLiteral("project root with spaces"));
    const QString executableDirectory = childPath(
        projectDirectory,
        QStringLiteral("build/custom-output"));
    QVERIFY(QDir().mkpath(executableDirectory));

    const QString markerPath = childPath(
        executableDirectory,
        QStringLiteral(".player-development-root"));
    QVERIFY(writeTextFile(markerPath, QByteArray("../..\n")));

    const RuntimePaths paths = RuntimePaths::resolve(
        RuntimePaths::Mode::Installed,
        executableDirectory);

    QCOMPARE(paths.mode(), RuntimePaths::Mode::Installed);
    QCOMPARE(paths.executableDirectory(), executableDirectory);
    QCOMPARE(paths.logDirectory(), projectDirectory);
}

void RuntimePathsTest::developmentMarkerOverridesPortableMode()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString projectDirectory = childPath(temporaryDirectory.path(), QStringLiteral("project"));
    const QString executableDirectory = childPath(
        projectDirectory,
        QStringLiteral("build/custom-output"));
    QVERIFY(QDir().mkpath(executableDirectory));

    QVERIFY(writeTextFile(
        childPath(executableDirectory, QStringLiteral(".player-development-root")),
        QByteArray("../..\n")));
    QVERIFY(writeTextFile(
        childPath(executableDirectory, QStringLiteral("portable.flag")),
        QByteArray()));

    const RuntimePaths::Mode detectedMode = RuntimePaths::detectMode(executableDirectory);
    QCOMPARE(detectedMode, RuntimePaths::Mode::Installed);

    const RuntimePaths paths = RuntimePaths::resolve(detectedMode, executableDirectory);
    QCOMPARE(paths.logDirectory(), projectDirectory);
}

void RuntimePathsTest::invalidDevelopmentMarkerFallsBackToPortableMode()
{
    QTemporaryDir executableDirectory;
    QVERIFY(executableDirectory.isValid());

    QVERIFY(writeTextFile(
        childPath(executableDirectory.path(), QStringLiteral(".player-development-root")),
        QDir::rootPath().toUtf8()));
    QVERIFY(writeTextFile(
        childPath(executableDirectory.path(), QStringLiteral("portable.flag")),
        QByteArray()));

    QCOMPARE(
        RuntimePaths::detectMode(executableDirectory.path()),
        RuntimePaths::Mode::Portable);
}

void RuntimePathsTest::portableModeStaysBesideExecutable()
{
    QTemporaryDir executableDirectory;
    QVERIFY(executableDirectory.isValid());

    const RuntimePaths paths = RuntimePaths::resolve(
        RuntimePaths::Mode::Portable,
        executableDirectory.path());
    const QString executableRoot = cleanPath(executableDirectory.path());

    QCOMPARE(paths.mode(), RuntimePaths::Mode::Portable);
    QCOMPARE(paths.executableDirectory(), executableRoot);
    QCOMPARE(paths.configDirectory(), childPath(executableRoot, QStringLiteral("config")));
    QCOMPARE(paths.dataDirectory(), childPath(executableRoot, QStringLiteral("data")));
    QCOMPARE(paths.logDirectory(), childPath(executableRoot, QStringLiteral("logs")));
    QCOMPARE(paths.screenshotDirectory(), childPath(executableRoot, QStringLiteral("screenshots")));

    QVERIFY(!QFileInfo(paths.configDirectory()).exists());
    QVERIFY(!QFileInfo(paths.dataDirectory()).exists());
    QVERIFY(!QFileInfo(paths.logDirectory()).exists());
    QVERIFY(!QFileInfo(paths.screenshotDirectory()).exists());
}

void RuntimePathsTest::portableMarkerControlsAutomaticModeDetection()
{
    QTemporaryDir executableDirectory;
    QVERIFY(executableDirectory.isValid());

    QCOMPARE(
        RuntimePaths::detectMode(executableDirectory.path()),
        RuntimePaths::Mode::Installed);

    QFile marker(QDir(executableDirectory.path()).filePath(QStringLiteral("portable.flag")));
    QVERIFY(marker.open(QIODevice::WriteOnly));
    marker.close();

    QCOMPARE(
        RuntimePaths::detectMode(executableDirectory.path()),
        RuntimePaths::Mode::Portable);

    QVERIFY(marker.remove());
    QCOMPARE(
        RuntimePaths::detectMode(executableDirectory.path()),
        RuntimePaths::Mode::Installed);
}

} // namespace player::app

QTEST_GUILESS_MAIN(player::app::RuntimePathsTest)

#include "runtime_paths_test.moc"
