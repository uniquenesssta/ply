#pragma once

#include "playback/infrastructure/mpv/commands/mpv_command_request.h"
#include "playback/infrastructure/mpv/events/mpv_event.h"
#include "playback/infrastructure/mpv/properties/mpv_property_registry.h"

#include <QList>
#include <QString>
#include <QtGlobal>

#include <optional>
#include <variant>

namespace player::tools::playback_probe {

struct ProbeSubmitCommandStep final
{
    quint64 requestId = 0;
    player::playback::mpv::MpvCommandRequest request;
    QString label;
};

struct ProbeWaitCommandReplyStep final
{
    quint64 requestId = 0;
    QString label;
};

struct ProbeWaitEventStep final
{
    player::playback::mpv::MpvEventType type = player::playback::mpv::MpvEventType::Unknown;
    std::optional<player::playback::mpv::MpvEndFileReason> endReason;
    QString label;
};

struct ProbeWaitFlagPropertyStep final
{
    player::playback::mpv::MpvPropertyId propertyId;
    bool expectedValue = false;
    QString label;
};

struct ProbeWaitStringPropertyStep final
{
    player::playback::mpv::MpvPropertyId propertyId;
    QString expectedValue;
    QString label;
};

struct ProbeEventBarrierStep final
{
    QString label;
};

struct ProbeShutdownStep final
{
    QString label;
};

using PlaybackProbeStep = std::variant<
    ProbeSubmitCommandStep,
    ProbeWaitCommandReplyStep,
    ProbeWaitEventStep,
    ProbeWaitFlagPropertyStep,
    ProbeWaitStringPropertyStep,
    ProbeEventBarrierStep,
    ProbeShutdownStep>;

struct PlaybackProbeScenario final
{
    QString id;
    QString description;
    QList<PlaybackProbeStep> steps;
};

} // namespace player::tools::playback_probe
