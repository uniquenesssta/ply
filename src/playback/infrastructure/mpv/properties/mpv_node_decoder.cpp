#include "playback/infrastructure/mpv/properties/mpv_node_decoder.h"

#include <mpv/client.h>

#include <QByteArray>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <limits>

namespace player::playback::mpv {
namespace {

constexpr int kMaximumNodeDepth = 64;

std::optional<QVariant> decodeNode(
    const mpv_node& node,
    int depth,
    QString* errorMessage)
{
    if (depth > kMaximumNodeDepth) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("mpv node nesting exceeds the supported depth.");
        }
        return std::nullopt;
    }

    switch (node.format) {
    case MPV_FORMAT_NONE:
        return QVariant{};
    case MPV_FORMAT_STRING:
        if (node.u.string == nullptr) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("mpv string node contains a null string pointer.");
            }
            return std::nullopt;
        }
        return QVariant{QString::fromUtf8(node.u.string)};
    case MPV_FORMAT_FLAG:
        return QVariant{node.u.flag != 0};
    case MPV_FORMAT_INT64:
        return QVariant::fromValue(static_cast<qlonglong>(node.u.int64));
    case MPV_FORMAT_DOUBLE:
        return QVariant{node.u.double_};
    case MPV_FORMAT_NODE_ARRAY: {
        if (node.u.list == nullptr) {
            return QVariant{QVariantList{}};
        }
        if (node.u.list->num < 0 || (node.u.list->num > 0 && node.u.list->values == nullptr)) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("mpv node array has an invalid list shape.");
            }
            return std::nullopt;
        }

        QVariantList values;
        values.reserve(node.u.list->num);
        for (int index = 0; index < node.u.list->num; ++index) {
            auto decoded = decodeNode(node.u.list->values[index], depth + 1, errorMessage);
            if (!decoded.has_value()) {
                return std::nullopt;
            }
            values.append(*decoded);
        }
        return QVariant{values};
    }
    case MPV_FORMAT_NODE_MAP: {
        if (node.u.list == nullptr) {
            return QVariant{QVariantMap{}};
        }
        if (node.u.list->num < 0
            || (node.u.list->num > 0
                && (node.u.list->values == nullptr || node.u.list->keys == nullptr))) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("mpv node map has an invalid list shape.");
            }
            return std::nullopt;
        }

        QVariantMap values;
        for (int index = 0; index < node.u.list->num; ++index) {
            if (node.u.list->keys[index] == nullptr) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("mpv node map contains a null key.");
                }
                return std::nullopt;
            }

            auto decoded = decodeNode(node.u.list->values[index], depth + 1, errorMessage);
            if (!decoded.has_value()) {
                return std::nullopt;
            }
            values.insert(QString::fromUtf8(node.u.list->keys[index]), *decoded);
        }
        return QVariant{values};
    }
    case MPV_FORMAT_BYTE_ARRAY:
        if (node.u.ba == nullptr) {
            return QVariant{QByteArray{}};
        }
        if (node.u.ba->size > static_cast<size_t>(std::numeric_limits<qsizetype>::max())) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("mpv byte-array node is too large to copy into Qt storage.");
            }
            return std::nullopt;
        }
        if (node.u.ba->size > 0 && node.u.ba->data == nullptr) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("mpv byte-array node has a null data pointer.");
            }
            return std::nullopt;
        }
        return QVariant{QByteArray(
            static_cast<const char*>(node.u.ba->data),
            static_cast<qsizetype>(node.u.ba->size))};
    default:
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unsupported mpv node format: %1.")
                                .arg(static_cast<int>(node.format));
        }
        return std::nullopt;
    }
}

} // namespace

std::optional<QVariant> MpvNodeDecoder::decode(
    const mpv_node& node,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return decodeNode(node, 0, errorMessage);
}

} // namespace player::playback::mpv
