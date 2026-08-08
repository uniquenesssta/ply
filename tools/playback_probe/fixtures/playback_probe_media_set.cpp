#include "playback_probe_media_set.h"

#include <QByteArray>
#include <QFile>
#include <QIODevice>

#include <limits>

namespace player::tools::playback_probe {
namespace {

void appendLittleEndian16(QByteArray& bytes, quint16 value)
{
    bytes.append(static_cast<char>(value & 0xffU));
    bytes.append(static_cast<char>((value >> 8U) & 0xffU));
}

void appendLittleEndian32(QByteArray& bytes, quint32 value)
{
    bytes.append(static_cast<char>(value & 0xffU));
    bytes.append(static_cast<char>((value >> 8U) & 0xffU));
    bytes.append(static_cast<char>((value >> 16U) & 0xffU));
    bytes.append(static_cast<char>((value >> 24U) & 0xffU));
}

bool writeSilentPcmWav(const QString& path, quint32 durationMs, QString* errorMessage)
{
    constexpr quint16 kChannels = 1;
    constexpr quint32 kSampleRate = 8000;
    constexpr quint16 kBitsPerSample = 16;
    constexpr quint16 kBlockAlign = kChannels * (kBitsPerSample / 8U);
    constexpr quint32 kByteRate = kSampleRate * kBlockAlign;

    const quint64 sampleCount64 =
        (static_cast<quint64>(kSampleRate) * static_cast<quint64>(durationMs)) / 1000U;
    const quint64 dataSize64 = sampleCount64 * kBlockAlign;
    if (sampleCount64 == 0U || dataSize64 > std::numeric_limits<quint32>::max()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Generated probe WAV duration is outside the supported range.");
        }
        return false;
    }

    const quint32 dataSize = static_cast<quint32>(dataSize64);
    QByteArray bytes;
    bytes.reserve(static_cast<qsizetype>(44U + dataSize));
    bytes.append("RIFF", 4);
    appendLittleEndian32(bytes, 36U + dataSize);
    bytes.append("WAVE", 4);
    bytes.append("fmt ", 4);
    appendLittleEndian32(bytes, 16U);
    appendLittleEndian16(bytes, 1U);
    appendLittleEndian16(bytes, kChannels);
    appendLittleEndian32(bytes, kSampleRate);
    appendLittleEndian32(bytes, kByteRate);
    appendLittleEndian16(bytes, kBlockAlign);
    appendLittleEndian16(bytes, kBitsPerSample);
    bytes.append("data", 4);
    appendLittleEndian32(bytes, dataSize);
    bytes.append(QByteArray(static_cast<qsizetype>(dataSize), '\0'));

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to create generated probe media '%1': %2")
                                .arg(path, file.errorString());
        }
        return false;
    }

    const qint64 written = file.write(bytes);
    if (written != bytes.size()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to fully write generated probe media '%1': %2")
                                .arg(path, file.errorString());
        }
        return false;
    }

    return true;
}

} // namespace

bool PlaybackProbeMediaSet::create(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!mediaA_.isEmpty()) {
        return true;
    }

    if (!directory_.isValid()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to create the playback probe temporary directory.");
        }
        return false;
    }

    mediaA_ = directory_.filePath(QStringLiteral("matrix-a.wav"));
    mediaB_ = directory_.filePath(QStringLiteral("matrix-b.wav"));
    eofMedia_ = directory_.filePath(QStringLiteral("matrix-eof.wav"));
    missingMedia_ = directory_.filePath(QStringLiteral("matrix-missing.wav"));

    QString error;
    if (!writeSilentPcmWav(mediaA_, 4000U, &error)
        || !writeSilentPcmWav(mediaB_, 5000U, &error)
        || !writeSilentPcmWav(eofMedia_, 350U, &error)) {
        mediaA_.clear();
        mediaB_.clear();
        eofMedia_.clear();
        missingMedia_.clear();
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    (void)QFile::remove(missingMedia_);
    return true;
}

const QString& PlaybackProbeMediaSet::mediaA() const noexcept
{
    return mediaA_;
}

const QString& PlaybackProbeMediaSet::mediaB() const noexcept
{
    return mediaB_;
}

const QString& PlaybackProbeMediaSet::eofMedia() const noexcept
{
    return eofMedia_;
}

const QString& PlaybackProbeMediaSet::missingMedia() const noexcept
{
    return missingMedia_;
}

} // namespace player::tools::playback_probe
