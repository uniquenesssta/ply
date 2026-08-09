#include "session_test_media.h"

#include <QByteArray>
#include <QDataStream>
#include <QFile>

namespace player::playback::application::test_support {

bool writeSilentPcmWav(
    const QString& path,
    int durationMilliseconds,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    if (durationMilliseconds <= 0) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("WAV duration must be positive.");
        }
        return false;
    }

    constexpr quint32 sampleRate = 8000;
    constexpr quint16 channels = 1;
    constexpr quint16 bitsPerSample = 16;
    constexpr quint16 bytesPerSample = bitsPerSample / 8;
    constexpr quint16 blockAlign = channels * bytesPerSample;
    constexpr quint32 byteRate = sampleRate * blockAlign;

    const quint32 sampleCount = static_cast<quint32>(
        (static_cast<quint64>(sampleRate) * static_cast<quint64>(durationMilliseconds)) / 1000ULL);
    const quint32 dataSize = sampleCount * blockAlign;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData("RIFF", 4);
    stream << quint32{36U + dataSize};
    stream.writeRawData("WAVE", 4);
    stream.writeRawData("fmt ", 4);
    stream << quint32{16U};
    stream << quint16{1U};
    stream << channels;
    stream << sampleRate;
    stream << byteRate;
    stream << blockAlign;
    stream << bitsPerSample;
    stream.writeRawData("data", 4);
    stream << dataSize;

    QByteArray silence(static_cast<qsizetype>(dataSize), '\0');
    const int bytesToWrite = static_cast<int>(silence.size());
    if (stream.writeRawData(silence.constData(), bytesToWrite) != bytesToWrite
        || stream.status() != QDataStream::Ok) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to write silent WAV fixture.");
        }
        return false;
    }

    return true;
}

} // namespace player::playback::application::test_support
