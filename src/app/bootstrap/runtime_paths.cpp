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

QString developmentProjectDirectory(const QString& executableDirectory)
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

    // Development builds always use the repository-root runtime policy. A stale
    // portable.flag beside a build artifact must not redirect development logs
    // into the generated build tree.
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
