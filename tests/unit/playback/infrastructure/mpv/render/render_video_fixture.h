#pragma once

#include <mpv/client.h>

#include <QByteArray>
#include <QFile>
#include <QString>

#include <thread>

namespace player::test::render {

enum class GeneratedY4mPattern
{
    AnimatedGray,
    BrightTopDarkBottom,
};

inline bool writeGeneratedY4mVideo(
    const QString& path,
    int frameCount,
    GeneratedY4mPattern pattern,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    const QByteArray header = QByteArrayLiteral("YUV4MPEG2 W16 H16 F30:1 Ip A1:1 C420jpeg\n");
    if (file.write(header) != header.size()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to write the generated Y4M header.");
        }
        return false;
    }

    QByteArray yPlane(16 * 16, '\0');
    const QByteArray uPlane(8 * 8, static_cast<char>(128));
    const QByteArray vPlane(8 * 8, static_cast<char>(128));
    const QByteArray frameHeader = QByteArrayLiteral("FRAME\n");

    for (int frame = 0; frame < frameCount; ++frame) {
        if (pattern == GeneratedY4mPattern::AnimatedGray) {
            yPlane.fill(static_cast<char>(16 + ((frame * 7) % 200)));
        } else {
            for (int row = 0; row < 16; ++row) {
                const char luma = row < 8
                    ? static_cast<char>(220)
                    : static_cast<char>(40);
                for (int column = 0; column < 16; ++column) {
                    yPlane[(row * 16) + column] = luma;
                }
            }
        }

        if (file.write(frameHeader) != frameHeader.size()
            || file.write(yPlane) != yPlane.size()
            || file.write(uPlane) != uPlane.size()
            || file.write(vPlane) != vPlane.size()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Unable to write generated Y4M frame %1.").arg(frame);
            }
            return false;
        }
    }

    return true;
}

inline int loadFileOnWorkerThread(mpv_handle* handle, const QString& path)
{
    int result = MPV_ERROR_GENERIC;
    const QByteArray source = path.toUtf8();

    std::thread commandThread([handle, source, &result] {
        const char* command[] = {
            "loadfile",
            source.constData(),
            nullptr,
        };
        result = mpv_command(handle, command);
    });
    commandThread.join();

    return result;
}

inline int setPauseOnWorkerThread(mpv_handle* handle, bool paused)
{
    int result = MPV_ERROR_GENERIC;

    std::thread commandThread([handle, paused, &result] {
        int pauseFlag = paused ? 1 : 0;
        result = mpv_set_property(handle, "pause", MPV_FORMAT_FLAG, &pauseFlag);
    });
    commandThread.join();

    return result;
}

inline int stopPlaybackOnWorkerThread(mpv_handle* handle)
{
    int result = MPV_ERROR_GENERIC;

    std::thread commandThread([handle, &result] {
        const char* command[] = {
            "stop",
            nullptr,
        };
        result = mpv_command(handle, command);
    });
    commandThread.join();

    return result;
}

} // namespace player::test::render
