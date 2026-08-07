#include "playback/infrastructure/mpv/runtime/mpv_runtime_manifest.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace player::playback::mpv {
namespace {

QString readRequiredString(
    const QJsonObject& object,
    const QString& key,
    const QString& fieldPath,
    QString* errorMessage)
{
    const QJsonValue value = object.value(key);
    if (!value.isString() || value.toString().trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("libmpv dependency manifest field '%1' is missing or empty.")
                                .arg(fieldPath);
        }
        return {};
    }

    return value.toString().trimmed();
}

} // namespace

bool MpvRuntimeManifest::load(
    const QString& manifestPath,
    MpvRuntimeManifestInfo& manifestInfo,
    QString* errorMessage)
{
    manifestInfo = {};

    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to open staged libmpv dependency manifest '%1': %2")
                                .arg(manifestPath, file.errorString());
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Invalid libmpv dependency manifest '%1': %2")
                                .arg(manifestPath, parseError.errorString());
        }
        return false;
    }

    const QJsonObject root = document.object();
    const QJsonValue mpvValue = root.value(QStringLiteral("mpv"));
    const QJsonValue ffmpegValue = root.value(QStringLiteral("ffmpeg"));
    if (!mpvValue.isObject() || !ffmpegValue.isObject()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "libmpv dependency manifest must contain object fields 'mpv' and 'ffmpeg'.");
        }
        return false;
    }

    const QJsonObject mpvObject = mpvValue.toObject();
    const QJsonObject ffmpegObject = ffmpegValue.toObject();

    manifestInfo.mpvVersion = readRequiredString(
        mpvObject, QStringLiteral("version"), QStringLiteral("mpv.version"), errorMessage);
    if (manifestInfo.mpvVersion.isEmpty()) {
        return false;
    }

    manifestInfo.mpvTag = readRequiredString(
        mpvObject, QStringLiteral("tag"), QStringLiteral("mpv.tag"), errorMessage);
    if (manifestInfo.mpvTag.isEmpty()) {
        return false;
    }

    manifestInfo.mpvCommit = readRequiredString(
        mpvObject, QStringLiteral("commit"), QStringLiteral("mpv.commit"), errorMessage);
    if (manifestInfo.mpvCommit.isEmpty()) {
        return false;
    }

    manifestInfo.ffmpegVersion = readRequiredString(
        ffmpegObject, QStringLiteral("version"), QStringLiteral("ffmpeg.version"), errorMessage);
    return !manifestInfo.ffmpegVersion.isEmpty();
}

} // namespace player::playback::mpv
