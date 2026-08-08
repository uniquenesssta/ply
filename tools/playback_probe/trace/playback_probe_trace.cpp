#include "playback_probe_trace.h"

#include "playback/infrastructure/mpv/properties/mpv_property_change.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <QMetaType>
#include <QTextStream>
#include <QVariantList>
#include <QVariantMap>

#include <utility>
#include <variant>

namespace player::tools::playback_probe {
namespace {

QString oneLine(QString value)
{
    value.replace(QLatin1Char('\r'), QLatin1Char(' '));
    value.replace(QLatin1Char('\n'), QLatin1Char(' '));
    return value.trimmed();
}

QString endReasonName(player::playback::mpv::MpvEndFileReason reason)
{
    using player::playback::mpv::MpvEndFileReason;

    switch (reason) {
    case MpvEndFileReason::Eof:
        return QStringLiteral("eof");
    case MpvEndFileReason::Stop:
        return QStringLiteral("stop");
    case MpvEndFileReason::Quit:
        return QStringLiteral("quit");
    case MpvEndFileReason::Error:
        return QStringLiteral("error");
    case MpvEndFileReason::Redirect:
        return QStringLiteral("redirect");
    case MpvEndFileReason::Unknown:
        return QStringLiteral("unknown");
    }

    return QStringLiteral("unknown");
}

QString formatPropertyValue(const player::playback::mpv::MpvPropertyValue& value)
{
    if (std::holds_alternative<std::monostate>(value)) {
        return QStringLiteral("<unavailable>");
    }
    if (const bool* flag = std::get_if<bool>(&value)) {
        return *flag ? QStringLiteral("true") : QStringLiteral("false");
    }
    if (const double* number = std::get_if<double>(&value)) {
        return QString::number(*number, 'g', 12);
    }
    if (const QString* text = std::get_if<QString>(&value)) {
        return QStringLiteral("\"") + oneLine(*text) + QStringLiteral("\"");
    }

    const QVariant* node = std::get_if<QVariant>(&value);
    if (node == nullptr || !node->isValid()) {
        return QStringLiteral("<node:none>");
    }
    if (node->metaType().id() == QMetaType::QVariantList) {
        return QStringLiteral("<node:list size=%1>").arg(node->toList().size());
    }
    if (node->metaType().id() == QMetaType::QVariantMap) {
        return QStringLiteral("<node:map size=%1>").arg(node->toMap().size());
    }
    return QStringLiteral("<node:%1>")
        .arg(QString::fromLatin1(node->typeName() == nullptr ? "unknown" : node->typeName()));
}

} // namespace

void PlaybackProbeTrace::reset(const QString& scenarioId)
{
    scenarioId_ = scenarioId;
    entries_.clear();
    truncated_ = false;
}

void PlaybackProbeTrace::recordCommand(quint64 requestId, const QString& label)
{
    append(QStringLiteral("command-submit id=%1 %2").arg(requestId).arg(oneLine(label)));
}

void PlaybackProbeTrace::recordMarker(const QString& marker)
{
    append(QStringLiteral("marker %1").arg(oneLine(marker)));
}

void PlaybackProbeTrace::recordEvent(const player::playback::mpv::MpvEvent& event)
{
    using namespace player::playback::mpv;

    switch (event.type) {
    case MpvEventType::StartFile: {
        const auto* data = std::get_if<MpvStartFileData>(&event.payload);
        append(data == nullptr
                   ? QStringLiteral("event start-file payload=<missing>")
                   : QStringLiteral("event start-file playlist-entry=%1")
                         .arg(data->playlistEntryId));
        return;
    }
    case MpvEventType::FileLoaded:
        append(QStringLiteral("event file-loaded"));
        return;
    case MpvEventType::EndFile: {
        const auto* data = std::get_if<MpvEndFileData>(&event.payload);
        if (data == nullptr) {
            append(QStringLiteral("event end-file payload=<missing>"));
            return;
        }
        append(QStringLiteral(
                   "event end-file reason=%1 raw-reason=%2 error=%3 playlist-entry=%4 insert-id=%5 insert-count=%6")
                   .arg(endReasonName(data->reason))
                   .arg(data->rawReason)
                   .arg(event.error.rawCode)
                   .arg(data->playlistEntryId)
                   .arg(data->playlistInsertId)
                   .arg(data->playlistInsertNumEntries));
        return;
    }
    case MpvEventType::CommandReply:
        append(QStringLiteral("event command-reply id=%1 error=%2 message=%3")
                   .arg(event.replyUserdata)
                   .arg(event.error.rawCode)
                   .arg(oneLine(event.error.message)));
        return;
    case MpvEventType::PropertyChange: {
        const auto* change = std::get_if<MpvPropertyChange>(&event.payload);
        if (change == nullptr) {
            append(QStringLiteral("event property-change payload=<missing>"));
            return;
        }
        const MpvPropertyDefinition* definition = MpvPropertyRegistry::findById(change->id);
        const QString name = definition == nullptr
                                 ? QStringLiteral("unknown")
                                 : QString::fromUtf8(definition->name);
        append(QStringLiteral("event property-change name=%1 value=%2")
                   .arg(name, formatPropertyValue(change->value)));
        return;
    }
    case MpvEventType::LogMessage: {
        const auto* log = std::get_if<MpvLogMessageData>(&event.payload);
        append(log == nullptr
                   ? QStringLiteral("event log-message payload=<missing>")
                   : QStringLiteral("event log-message level=%1 prefix=%2 text=%3")
                         .arg(oneLine(log->level), oneLine(log->prefix), oneLine(log->text).left(160)));
        return;
    }
    case MpvEventType::Shutdown:
        append(QStringLiteral("event shutdown"));
        return;
    case MpvEventType::Unknown: {
        const auto* unknown = std::get_if<MpvUnknownEventData>(&event.payload);
        append(QStringLiteral("event unknown raw-id=%1")
                   .arg(unknown == nullptr ? -1 : unknown->rawEventId));
        return;
    }
    case MpvEventType::DecodeFailure: {
        const auto* failure = std::get_if<MpvDecodeFailureData>(&event.payload);
        append(failure == nullptr
                   ? QStringLiteral("event decode-failure payload=<missing>")
                   : QStringLiteral("event decode-failure raw-id=%1 diagnostic=%2")
                         .arg(failure->rawEventId)
                         .arg(oneLine(failure->diagnostic)));
        return;
    }
    }
}

void PlaybackProbeTrace::dump() const
{
    QTextStream output(stdout);
    for (qsizetype index = 0; index < entries_.size(); ++index) {
        output << "[playback_probe][trace][" << scenarioId_ << "] "
               << QString::number(index + 1).rightJustified(3, QLatin1Char('0')) << ' '
               << entries_.at(index) << Qt::endl;
    }
    if (truncated_) {
        output << "[playback_probe][trace][" << scenarioId_
               << "] ... trace truncated after " << kMaximumEntries << " entries" << Qt::endl;
    }
}

void PlaybackProbeTrace::append(QString entry)
{
    if (entries_.size() >= kMaximumEntries) {
        truncated_ = true;
        return;
    }
    entries_.append(std::move(entry));
}

} // namespace player::tools::playback_probe
