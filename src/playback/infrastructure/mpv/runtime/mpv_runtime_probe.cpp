#include "playback/infrastructure/mpv/runtime/mpv_runtime_probe.h"

#include <mpv/client.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <Windows.h>

#include <cstddef>
#include <vector>
#endif

namespace player::playback::mpv {
namespace {

constexpr int apiMajor(unsigned long version) noexcept
{
    return static_cast<int>(version >> 16U);
}

constexpr int apiMinor(unsigned long version) noexcept
{
    return static_cast<int>(version & 0xFFFFU);
}

bool verifyIdentity(
    const QString& name,
    const QString& actual,
    const QString& expected,
    QString* errorMessage)
{
    if (actual == expected) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("libmpv manifest %1 mismatch. Expected '%2', found '%3'.")
                            .arg(name, expected, actual);
    }
    return false;
}

QString loadedRuntimeLibraryPath(QString* errorMessage)
{
    const QString runtimeFileName = QStringLiteral(PLAYER_LIBMPV_RUNTIME_FILENAME);

#ifdef Q_OS_WIN
    const HMODULE module = GetModuleHandleW(
        reinterpret_cast<LPCWSTR>(runtimeFileName.utf16()));
    if (module == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("The linked libmpv runtime '%1' is not loaded in the current process.")
                                .arg(runtimeFileName);
        }
        return {};
    }

    std::vector<wchar_t> pathBuffer(32768, L'\0');
    const DWORD length = GetModuleFileNameW(
        module,
        pathBuffer.data(),
        static_cast<DWORD>(pathBuffer.size()));
    if (length == 0 || static_cast<std::size_t>(length) >= pathBuffer.size()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to resolve the loaded libmpv DLL path for '%1'.")
                                .arg(runtimeFileName);
        }
        return {};
    }

    return QDir::cleanPath(QString::fromWCharArray(pathBuffer.data(), static_cast<int>(length)));
#else
    const QString stagedPath = QDir(QCoreApplication::applicationDirPath()).filePath(runtimeFileName);
    const QFileInfo stagedInfo(stagedPath);
    if (!stagedInfo.exists()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("The staged libmpv runtime was not found at '%1'.")
                                .arg(stagedPath);
        }
        return {};
    }
    return stagedInfo.canonicalFilePath();
#endif
}

} // namespace

bool MpvRuntimeProbe::probe(MpvRuntimeInfo& runtimeInfo, QString* errorMessage)
{
    runtimeInfo = {};

    const unsigned long runtimeApiVersion = mpv_client_api_version();
    const unsigned long compiledApiVersion = MPV_CLIENT_API_VERSION;
    runtimeInfo.clientApiMajor = apiMajor(runtimeApiVersion);
    runtimeInfo.clientApiMinor = apiMinor(runtimeApiVersion);

    const int compiledApiMajor = apiMajor(compiledApiVersion);
    const int compiledApiMinor = apiMinor(compiledApiVersion);
    if (runtimeInfo.clientApiMajor != compiledApiMajor ||
        runtimeInfo.clientApiMinor < compiledApiMinor) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "libmpv client API mismatch. Headers require %1.%2, runtime provides %3.%4.")
                                .arg(compiledApiMajor)
                                .arg(compiledApiMinor)
                                .arg(runtimeInfo.clientApiMajor)
                                .arg(runtimeInfo.clientApiMinor);
        }
        return false;
    }

    runtimeInfo.runtimeLibraryPath = loadedRuntimeLibraryPath(errorMessage);
    if (runtimeInfo.runtimeLibraryPath.isEmpty()) {
        return false;
    }

    const QString applicationDirectory = QDir(QCoreApplication::applicationDirPath()).canonicalPath();
    const QString runtimeDirectory = QFileInfo(runtimeInfo.runtimeLibraryPath).absoluteDir().canonicalPath();
    if (applicationDirectory.isEmpty() || runtimeDirectory.isEmpty() ||
        applicationDirectory.compare(runtimeDirectory, Qt::CaseInsensitive) != 0) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "libmpv was loaded from '%1' instead of the staged application directory '%2'.")
                                .arg(runtimeInfo.runtimeLibraryPath, QCoreApplication::applicationDirPath());
        }
        return false;
    }

    runtimeInfo.manifestPath = QDir(QCoreApplication::applicationDirPath())
                                   .filePath(QStringLiteral(PLAYER_LIBMPV_STAGED_MANIFEST_RELATIVE_PATH));
    QString manifestError;
    if (!MpvRuntimeManifest::load(runtimeInfo.manifestPath, runtimeInfo.manifest, &manifestError)) {
        if (errorMessage != nullptr) {
            *errorMessage = manifestError;
        }
        return false;
    }

    if (!verifyIdentity(
            QStringLiteral("mpv.version"),
            runtimeInfo.manifest.mpvVersion,
            QStringLiteral(PLAYER_EXPECTED_MPV_VERSION),
            errorMessage) ||
        !verifyIdentity(
            QStringLiteral("mpv.tag"),
            runtimeInfo.manifest.mpvTag,
            QStringLiteral(PLAYER_EXPECTED_MPV_TAG),
            errorMessage) ||
        !verifyIdentity(
            QStringLiteral("mpv.commit"),
            runtimeInfo.manifest.mpvCommit,
            QStringLiteral(PLAYER_EXPECTED_MPV_COMMIT),
            errorMessage) ||
        !verifyIdentity(
            QStringLiteral("ffmpeg.version"),
            runtimeInfo.manifest.ffmpegVersion,
            QStringLiteral(PLAYER_EXPECTED_FFMPEG_VERSION),
            errorMessage)) {
        return false;
    }

    return true;
}

} // namespace player::playback::mpv
