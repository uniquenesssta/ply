#include "playback/infrastructure/mpv/commands/mpv_command_encoder.h"

#include <QString>

#include <cmath>
#include <type_traits>
#include <utility>

namespace player::playback::mpv {
namespace {

std::optional<QList<QByteArray>> failEncoding(
    QString message,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        *errorMessage = std::move(message);
    }
    return std::nullopt;
}

QByteArray encodeNumber(double value)
{
    return QByteArray::number(value, 'g', 17);
}

} // namespace

std::optional<QList<QByteArray>> MpvCommandEncoder::encode(
    const MpvCommandRequest& request,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    return std::visit(
        [errorMessage](const auto& typedRequest) -> std::optional<QList<QByteArray>> {
            using Request = std::decay_t<decltype(typedRequest)>;

            if constexpr (std::is_same_v<Request, MpvLoadRequest>) {
                const QByteArray source = typedRequest.source.toUtf8();
                if (source.isEmpty() || source.contains('\0')) {
                    return failEncoding(
                        QStringLiteral("Load source must be non-empty UTF-8 text without embedded null bytes."),
                        errorMessage);
                }

                return QList<QByteArray>{
                    QByteArrayLiteral("loadfile"),
                    source,
                    QByteArrayLiteral("replace"),
                };
            } else if constexpr (std::is_same_v<Request, MpvPlayRequest>) {
                return QList<QByteArray>{
                    QByteArrayLiteral("set"),
                    QByteArrayLiteral("pause"),
                    QByteArrayLiteral("no"),
                };
            } else if constexpr (std::is_same_v<Request, MpvPauseRequest>) {
                return QList<QByteArray>{
                    QByteArrayLiteral("set"),
                    QByteArrayLiteral("pause"),
                    QByteArrayLiteral("yes"),
                };
            } else if constexpr (std::is_same_v<Request, MpvStopRequest>) {
                return QList<QByteArray>{QByteArrayLiteral("stop")};
            } else if constexpr (std::is_same_v<Request, MpvSeekRequest>) {
                if (!std::isfinite(typedRequest.seconds)) {
                    return failEncoding(
                        QStringLiteral("Seek seconds must be finite."),
                        errorMessage);
                }

                return QList<QByteArray>{
                    QByteArrayLiteral("seek"),
                    encodeNumber(typedRequest.seconds),
                    typedRequest.mode == MpvSeekMode::Absolute
                        ? QByteArrayLiteral("absolute+exact")
                        : QByteArrayLiteral("relative+exact"),
                };
            } else if constexpr (std::is_same_v<Request, MpvVolumeRequest>) {
                if (!std::isfinite(typedRequest.percent) || typedRequest.percent < 0.0) {
                    return failEncoding(
                        QStringLiteral("Volume percent must be finite and non-negative."),
                        errorMessage);
                }

                return QList<QByteArray>{
                    QByteArrayLiteral("set"),
                    QByteArrayLiteral("volume"),
                    encodeNumber(typedRequest.percent),
                };
            } else if constexpr (std::is_same_v<Request, MpvMuteRequest>) {
                return QList<QByteArray>{
                    QByteArrayLiteral("set"),
                    QByteArrayLiteral("mute"),
                    typedRequest.muted ? QByteArrayLiteral("yes") : QByteArrayLiteral("no"),
                };
            } else {
                static_assert(std::is_same_v<Request, MpvSpeedRequest>);
                if (!std::isfinite(typedRequest.rate) || typedRequest.rate <= 0.0) {
                    return failEncoding(
                        QStringLiteral("Playback speed must be finite and greater than zero."),
                        errorMessage);
                }

                return QList<QByteArray>{
                    QByteArrayLiteral("set"),
                    QByteArrayLiteral("speed"),
                    encodeNumber(typedRequest.rate),
                };
            }
        },
        request);
}

} // namespace player::playback::mpv
