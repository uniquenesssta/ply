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

} // namespace

class RuntimePathsTest final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void installedModeUsesStandardLocations();
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
