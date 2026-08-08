#include "app/bootstrap/runtime_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <utility>

namespace player::app {
namespace {

constexpr auto kPortableMarkerFileName = "portable.flag";
constexpr auto kBuildDirectoryName = "build";
constexpr auto kWindowsMsvcPresetPrefix = "windows-msvc-";

QString normalizedPath(QString path)
{
    if (path.isEmpty()) {
        return {};
    }

    return QDir::cleanPath(QDir::fromNativeSeparators(std::move(path)));
}

QString childPath(const QString& parent, const QString& child)
{
    if (parent.isEmpty()) {
        return {};
    }

    return normalizedPath(QDir(parent).filePath(child));
}

Qt::CaseSensitivity pathCaseSensitivity()
{
#ifdef Q_OS_WIN
    return Qt::CaseInsensitive;
#else
    return Qt::CaseSensitive;
#endif
}

bool isPathInsideDirectory(const QString& path, const QString& directory)
{
    const QString normalizedCandidate = normalizedPath(path);
    const QString normalizedDirectory = normalizedPath(directory);
    if (normalizedCandidate.isEmpty() || normalizedDirectory.isEmpty()) {
        return false;
    }

    if (normalizedCandidate.compare(normalizedDirectory, pathCaseSensitivity()) == 0) {
        return true;
    }

    const QString prefix = normalizedDirectory.endsWith(QLatin1Char('/'))
        ? normalizedDirectory
        : normalizedDirectory + QLatin1Char('/');
    return normalizedCandidate.startsWith(prefix, pathCaseSensitivity());
}

bool isProjectRootDirectory(const QDir& directory)
{
    return QFileInfo(directory.filePath(QStringLiteral("CMakeLists.txt"))).isFile()
        && QFileInfo(directory.filePath(QStringLiteral("ALL_AI_CODE.md"))).isFile()
        && QFileInfo(directory.filePath(QStringLiteral("AI_PROJECT_RULES.md"))).isFile()
        && QFileInfo(directory.filePath(QStringLiteral("src"))).isDir();
}

QString developmentProjectDirectoryFromWorkingDirectory(const QString& executableDirectory)
{
    QDir workingDirectory(QDir::currentPath());
    if (!isProjectRootDirectory(workingDirectory)) {
        return {};
    }

    const QString projectDirectory = normalizedPath(workingDirectory.absolutePath());
    const QString buildDirectory = childPath(projectDirectory, QString::fromLatin1(kBuildDirectoryName));
    if (!isPathInsideDirectory(executableDirectory, buildDirectory)) {
        return {};
    }

    return projectDirectory;
}

QString developmentProjectDirectoryFromAncestors(const QString& executableDirectory)
{
    const QString normalizedExecutableDirectory = normalizedPath(executableDirectory);
    if (normalizedExecutableDirectory.isEmpty()) {
        return {};
    }

    QDir directory(normalizedExecutableDirectory);
    for (int depth = 0; depth < 6; ++depth) {
        if (isProjectRootDirectory(directory)) {
            const QString projectDirectory = normalizedPath(directory.absolutePath());
            const QString buildDirectory = childPath(projectDirectory, QString::fromLatin1(kBuildDirectoryName));
            if (isPathInsideDirectory(normalizedExecutableDirectory, buildDirectory)) {
                return projectDirectory;
            }
        }

        if (!directory.cdUp()) {
            break;
        }
    }

    return {};
}

QString developmentProjectDirectoryFromLegacyLayout(const QString& executableDirectory)
{
    QDir directory(executableDirectory);
    if (!directory.dirName().startsWith(QString::fromLatin1(kWindowsMsvcPresetPrefix))) {
        return {};
    }

    if (!directory.cdUp() || directory.dirName() != QString::fromLatin1(kBuildDirectoryName)) {
        return {};
    }

    if (!directory.cdUp()) {
        return {};
    }

    return normalizedPath(directory.absolutePath());
}

QString developmentProjectDirectory(const QString& executableDirectory)
{
    const QString workingDirectoryProject =
        developmentProjectDirectoryFromWorkingDirectory(executableDirectory);
    if (!workingDirectoryProject.isEmpty()) {
        return workingDirectoryProject;
    }

    const QString ancestorProject = developmentProjectDirectoryFromAncestors(executableDirectory);
    if (!ancestorProject.isEmpty()) {
        return ancestorProject;
    }

    return developmentProjectDirectoryFromLegacyLayout(executableDirectory);
}

QString installedLogDirectory(
    const QString& executableDirectory,
    const QString& dataDirectory)
{
    const QString projectDirectory = developmentProjectDirectory(executableDirectory);
    if (!projectDirectory.isEmpty()) {
        return projectDirectory;
    }

    return childPath(dataDirectory, QStringLiteral("logs"));
}

QString installedScreenshotDirectory(const QString& dataDirectory)
{
    const QString picturesDirectory = normalizedPath(
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    const QString applicationName = QCoreApplication::applicationName().trimmed();

    if (picturesDirectory.isEmpty() || applicationName.isEmpty()) {
        return childPath(dataDirectory, QStringLiteral("screenshots"));
    }

    return childPath(picturesDirectory, applicationName);
}

} // namespace

RuntimePaths RuntimePaths::current()
{
    const QString executableDirectory = normalizedPath(QCoreApplication::applicationDirPath());
    return resolve(detectMode(executableDirectory), executableDirectory);
}

RuntimePaths RuntimePaths::resolve(Mode mode, const QString& executableDirectory)
{
    const QString normalizedExecutableDirectory = normalizedPath(executableDirectory);

    if (mode == Mode::Portable) {
        return RuntimePaths(
            mode,
            normalizedExecutableDirectory,
            childPath(normalizedExecutableDirectory, QStringLiteral("config")),
            childPath(normalizedExecutableDirectory, QStringLiteral("data")),
            childPath(normalizedExecutableDirectory, QStringLiteral("logs")),
            childPath(normalizedExecutableDirectory, QStringLiteral("screenshots")));
    }

    const QString configDirectory = normalizedPath(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString dataDirectory = normalizedPath(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));

    return RuntimePaths(
        mode,
        normalizedExecutableDirectory,
        configDirectory,
        dataDirectory,
        installedLogDirectory(normalizedExecutableDirectory, dataDirectory),
        installedScreenshotDirectory(dataDirectory));
}

RuntimePaths::Mode RuntimePaths::detectMode(const QString& executableDirectory)
{
    const QString normalizedExecutableDirectory = normalizedPath(executableDirectory);
    if (normalizedExecutableDirectory.isEmpty()) {
        return Mode::Installed;
    }

    // Development builds always use the repository-root runtime policy. Project
    // identity is resolved from the active repository or its build ancestry,
    // with the original preset-layout rule retained as a compatibility fallback.
    if (!developmentProjectDirectory(normalizedExecutableDirectory).isEmpty()) {
        return Mode::Installed;
    }

    const QString markerPath = QDir(normalizedExecutableDirectory).filePath(
        QString::fromLatin1(kPortableMarkerFileName));
    return QFileInfo(markerPath).isFile() ? Mode::Portable : Mode::Installed;
}

RuntimePaths::Mode RuntimePaths::mode() const noexcept
{
    return m_mode;
}

const QString& RuntimePaths::executableDirectory() const noexcept
{
    return m_executableDirectory;
}

const QString& RuntimePaths::configDirectory() const noexcept
{
    return m_configDirectory;
}

const QString& RuntimePaths::dataDirectory() const noexcept
{
    return m_dataDirectory;
}

const QString& RuntimePaths::logDirectory() const noexcept
{
    return m_logDirectory;
}

const QString& RuntimePaths::screenshotDirectory() const noexcept
{
    return m_screenshotDirectory;
}

RuntimePaths::RuntimePaths(
    Mode mode,
    QString executableDirectory,
    QString configDirectory,
    QString dataDirectory,
    QString logDirectory,
    QString screenshotDirectory)
    : m_mode(mode)
    , m_executableDirectory(std::move(executableDirectory))
    , m_configDirectory(std::move(configDirectory))
    , m_dataDirectory(std::move(dataDirectory))
    , m_logDirectory(std::move(logDirectory))
    , m_screenshotDirectory(std::move(screenshotDirectory))
{
}

} // namespace player::app
