#include "app/bootstrap/runtime_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include <utility>

#ifdef Q_OS_WIN
#include <Windows.h>

#include <cstddef>
#include <vector>
#endif

namespace player::app {
namespace {

constexpr auto kPortableMarkerFileName = "portable.flag";
constexpr auto kDevelopmentRootMarkerFileName = ".player-development-root";
constexpr auto kDevelopmentMarkerDirectoryName = "cmake";
constexpr auto kCanonicalDevelopmentRootRoute = "../../..";
constexpr auto kLegacyDevelopmentRootRoute = "../..";
constexpr auto kDevelopmentLogDirectoryName = "logs";

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

QString currentProcessExecutableFilePath()
{
#ifdef Q_OS_WIN
    std::vector<wchar_t> pathBuffer(32768, L'\0');
    const DWORD length = GetModuleFileNameW(
        nullptr,
        pathBuffer.data(),
        static_cast<DWORD>(pathBuffer.size()));
    if (length == 0 || static_cast<std::size_t>(length) >= pathBuffer.size()) {
        return {};
    }

    return normalizedPath(
        QString::fromWCharArray(pathBuffer.data(), static_cast<int>(length)));
#else
    if (QCoreApplication::instance() == nullptr) {
        return {};
    }

    return normalizedPath(QCoreApplication::applicationFilePath());
#endif
}

QString projectDirectoryFromDevelopmentMarker(
    const QString& markerDirectory,
    const QString& expectedRelativeRoot)
{
    const QString normalizedMarkerDirectory = normalizedPath(markerDirectory);
    if (normalizedMarkerDirectory.isEmpty()) {
        return {};
    }

    QFile marker(QDir(normalizedMarkerDirectory).filePath(
        QString::fromLatin1(kDevelopmentRootMarkerFileName)));
    if (!marker.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString relativeRoot = QString::fromUtf8(marker.readAll()).trimmed();
    if (relativeRoot != expectedRelativeRoot || QDir::isAbsolutePath(relativeRoot)) {
        return {};
    }

    const QString projectDirectory = normalizedPath(
        QDir(normalizedMarkerDirectory).absoluteFilePath(relativeRoot));
    if (projectDirectory.isEmpty() || !QFileInfo(projectDirectory).isDir()) {
        return {};
    }

    return projectDirectory;
}

QString developmentProjectDirectory(const QString& executableDirectory)
{
    const QString normalizedExecutableDirectory = normalizedPath(executableDirectory);
    if (normalizedExecutableDirectory.isEmpty()) {
        return {};
    }

    const QString canonicalMarkerDirectory = childPath(
        normalizedExecutableDirectory,
        QString::fromLatin1(kDevelopmentMarkerDirectoryName));
    const QString canonicalMarkerPath = childPath(
        canonicalMarkerDirectory,
        QString::fromLatin1(kDevelopmentRootMarkerFileName));

    if (QFileInfo(canonicalMarkerPath).isFile()) {
        return projectDirectoryFromDevelopmentMarker(
            canonicalMarkerDirectory,
            QString::fromLatin1(kCanonicalDevelopmentRootRoute));
    }

    return projectDirectoryFromDevelopmentMarker(
        normalizedExecutableDirectory,
        QString::fromLatin1(kLegacyDevelopmentRootRoute));
}

QString developmentLogDirectory(const QString& projectDirectory)
{
    const QString projectParentDirectory = normalizedPath(
        QDir(projectDirectory).absoluteFilePath(QStringLiteral("..")));
    return childPath(
        projectParentDirectory,
        QString::fromLatin1(kDevelopmentLogDirectoryName));
}

QString installedLogDirectory(
    const QString& executableDirectory,
    const QString& dataDirectory)
{
    const QString projectDirectory = developmentProjectDirectory(executableDirectory);
    if (!projectDirectory.isEmpty()) {
        return developmentLogDirectory(projectDirectory);
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

RuntimePaths RuntimePaths::fromCurrentProcessExecutable()
{
    return fromExecutableFilePath(currentProcessExecutableFilePath());
}

RuntimePaths RuntimePaths::fromExecutableFilePath(const QString& executableFilePath)
{
    if (executableFilePath.trimmed().isEmpty()) {
        return resolve(Mode::Installed, {});
    }

    const QFileInfo executableInfo(executableFilePath);
    const QString executableDirectory = normalizedPath(executableInfo.absolutePath());
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