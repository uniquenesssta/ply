#include "playback_probe_scenario_catalog.h"

#include "fixtures/playback_probe_media_set.h"

#include <utility>

namespace player::tools::playback_probe {
namespace {

using namespace player::playback::mpv;

ProbeSubmitCommandStep submit(
    quint64 requestId,
    MpvCommandRequest request,
    const QString& label)
{
    return ProbeSubmitCommandStep{requestId, std::move(request), label};
}

ProbeWaitCommandReplyStep reply(quint64 requestId, const QString& label)
{
    return ProbeWaitCommandReplyStep{requestId, label};
}

ProbeWaitEventStep event(MpvEventType type, const QString& label)
{
    return ProbeWaitEventStep{type, std::nullopt, label};
}

ProbeWaitEventStep endFile(MpvEndFileReason reason, const QString& label)
{
    return ProbeWaitEventStep{MpvEventType::EndFile, reason, label};
}

ProbeWaitFlagPropertyStep flag(MpvPropertyId id, bool value, const QString& label)
{
    return ProbeWaitFlagPropertyStep{id, value, label};
}

ProbeWaitStringPropertyStep stringProperty(
    MpvPropertyId id,
    const QString& value,
    const QString& label)
{
    return ProbeWaitStringPropertyStep{id, value, label};
}

ProbeEventBarrierStep barrier(const QString& label)
{
    return ProbeEventBarrierStep{label};
}

ProbeShutdownStep shutdown(const QString& label)
{
    return ProbeShutdownStep{label};
}

PlaybackProbeScenario transportScenario(const PlaybackProbeMediaSet& mediaSet)
{
    return PlaybackProbeScenario{
        QStringLiteral("transport-sequence"),
        QStringLiteral("load -> play -> pause -> seek -> play -> stop"),
        {
            submit(1101, MpvLoadRequest{mediaSet.mediaA()}, QStringLiteral("load A")),
            reply(1101, QStringLiteral("load A reply")),
            event(MpvEventType::FileLoaded, QStringLiteral("A file-loaded")),
            submit(1102, MpvPlayRequest{}, QStringLiteral("play")),
            reply(1102, QStringLiteral("play reply")),
            submit(1103, MpvPauseRequest{}, QStringLiteral("pause")),
            reply(1103, QStringLiteral("pause reply")),
            flag(MpvPropertyId::Pause, true, QStringLiteral("pause=true")),
            submit(
                1104,
                MpvSeekRequest{1.0, MpvSeekMode::Absolute},
                QStringLiteral("seek absolute 1.0s while paused")),
            reply(1104, QStringLiteral("seek reply")),
            submit(1105, MpvPlayRequest{}, QStringLiteral("play after seek")),
            reply(1105, QStringLiteral("second play reply")),
            barrier(QStringLiteral("discard pre-stop buffered events")),
            submit(1106, MpvStopRequest{}, QStringLiteral("stop")),
            reply(1106, QStringLiteral("stop reply")),
            endFile(MpvEndFileReason::Stop, QStringLiteral("end-file stop")),
        }};
}

PlaybackProbeScenario pausedSeekScenario(const PlaybackProbeMediaSet& mediaSet)
{
    return PlaybackProbeScenario{
        QStringLiteral("paused-seek"),
        QStringLiteral("seek while pause property is true"),
        {
            submit(2201, MpvLoadRequest{mediaSet.mediaA()}, QStringLiteral("load A")),
            reply(2201, QStringLiteral("load A reply")),
            event(MpvEventType::FileLoaded, QStringLiteral("A file-loaded")),
            submit(2202, MpvPauseRequest{}, QStringLiteral("pause")),
            reply(2202, QStringLiteral("pause reply")),
            flag(MpvPropertyId::Pause, true, QStringLiteral("pause=true before seek")),
            submit(
                2203,
                MpvSeekRequest{1.5, MpvSeekMode::Absolute},
                QStringLiteral("seek absolute 1.5s")),
            reply(2203, QStringLiteral("paused seek reply")),
            barrier(QStringLiteral("discard pre-stop buffered events")),
            submit(2204, MpvStopRequest{}, QStringLiteral("stop")),
            reply(2204, QStringLiteral("stop reply")),
            endFile(MpvEndFileReason::Stop, QStringLiteral("end-file stop")),
        }};
}

PlaybackProbeScenario consecutiveSeekScenario(const PlaybackProbeMediaSet& mediaSet)
{
    return PlaybackProbeScenario{
        QStringLiteral("consecutive-seek"),
        QStringLiteral("submit two different seeks without waiting between submissions"),
        {
            submit(3301, MpvLoadRequest{mediaSet.mediaB()}, QStringLiteral("load B")),
            reply(3301, QStringLiteral("load B reply")),
            event(MpvEventType::FileLoaded, QStringLiteral("B file-loaded")),
            submit(3302, MpvPauseRequest{}, QStringLiteral("pause")),
            reply(3302, QStringLiteral("pause reply")),
            flag(MpvPropertyId::Pause, true, QStringLiteral("pause=true before consecutive seeks")),
            submit(
                3303,
                MpvSeekRequest{0.75, MpvSeekMode::Absolute},
                QStringLiteral("seek #1 absolute 0.75s")),
            submit(
                3304,
                MpvSeekRequest{2.25, MpvSeekMode::Absolute},
                QStringLiteral("seek #2 absolute 2.25s")),
            reply(3303, QStringLiteral("seek #1 reply")),
            reply(3304, QStringLiteral("seek #2 reply")),
            barrier(QStringLiteral("discard pre-stop buffered events")),
            submit(3305, MpvStopRequest{}, QStringLiteral("stop")),
            reply(3305, QStringLiteral("stop reply")),
            endFile(MpvEndFileReason::Stop, QStringLiteral("end-file stop")),
        }};
}

PlaybackProbeScenario replacementScenario(const PlaybackProbeMediaSet& mediaSet)
{
    return PlaybackProbeScenario{
        QStringLiteral("replace-a-with-b"),
        QStringLiteral("submit load A and load B immediately and record the replacement sequence"),
        {
            submit(4401, MpvLoadRequest{mediaSet.mediaA()}, QStringLiteral("load A")),
            submit(4402, MpvLoadRequest{mediaSet.mediaB()}, QStringLiteral("load B immediately after A")),
            reply(4401, QStringLiteral("load A reply")),
            reply(4402, QStringLiteral("load B reply")),
            stringProperty(MpvPropertyId::Path, mediaSet.mediaB(), QStringLiteral("current path is B")),
            event(MpvEventType::FileLoaded, QStringLiteral("file-loaded observed during A/B replacement")),
            barrier(QStringLiteral("B established; discard replacement end events before explicit stop")),
            submit(4403, MpvStopRequest{}, QStringLiteral("stop B")),
            reply(4403, QStringLiteral("stop B reply")),
            endFile(MpvEndFileReason::Stop, QStringLiteral("B end-file stop")),
        }};
}

PlaybackProbeScenario endReasonScenario(const PlaybackProbeMediaSet& mediaSet)
{
    return PlaybackProbeScenario{
        QStringLiteral("end-reason-distinction"),
        QStringLiteral("distinguish natural EOF from explicit stop"),
        {
            submit(5501, MpvLoadRequest{mediaSet.eofMedia()}, QStringLiteral("load short EOF media")),
            reply(5501, QStringLiteral("EOF media load reply")),
            event(MpvEventType::FileLoaded, QStringLiteral("EOF media file-loaded")),
            endFile(MpvEndFileReason::Eof, QStringLiteral("natural end-file EOF")),
            submit(5502, MpvLoadRequest{mediaSet.mediaA()}, QStringLiteral("load A for stop comparison")),
            reply(5502, QStringLiteral("load A reply")),
            event(MpvEventType::FileLoaded, QStringLiteral("A file-loaded")),
            barrier(QStringLiteral("discard pre-stop buffered events")),
            submit(5503, MpvStopRequest{}, QStringLiteral("explicit stop")),
            reply(5503, QStringLiteral("explicit stop reply")),
            endFile(MpvEndFileReason::Stop, QStringLiteral("explicit end-file stop")),
        }};
}

PlaybackProbeScenario loadErrorScenario(const PlaybackProbeMediaSet& mediaSet)
{
    return PlaybackProbeScenario{
        QStringLiteral("load-error"),
        QStringLiteral("load a missing media source and require end-file error"),
        {
            submit(6601, MpvLoadRequest{mediaSet.missingMedia()}, QStringLiteral("load missing media")),
            reply(6601, QStringLiteral("missing media load command reply")),
            endFile(MpvEndFileReason::Error, QStringLiteral("end-file loading error")),
        }};
}

PlaybackProbeScenario shutdownLoadingScenario(const PlaybackProbeMediaSet& mediaSet)
{
    return PlaybackProbeScenario{
        QStringLiteral("shutdown-during-loading"),
        QStringLiteral("request runtime shutdown immediately after start-file"),
        {
            submit(7701, MpvLoadRequest{mediaSet.mediaB()}, QStringLiteral("load B")),
            event(MpvEventType::StartFile, QStringLiteral("start-file before shutdown")),
            shutdown(QStringLiteral("shutdown while load is in flight")),
        }};
}

PlaybackProbeScenario shutdownPlaybackScenario(const PlaybackProbeMediaSet& mediaSet)
{
    return PlaybackProbeScenario{
        QStringLiteral("shutdown-during-playback"),
        QStringLiteral("request runtime shutdown after file-loaded and play reply"),
        {
            submit(8801, MpvLoadRequest{mediaSet.mediaB()}, QStringLiteral("load B")),
            reply(8801, QStringLiteral("load B reply")),
            event(MpvEventType::FileLoaded, QStringLiteral("B file-loaded")),
            submit(8802, MpvPlayRequest{}, QStringLiteral("play B")),
            reply(8802, QStringLiteral("play B reply")),
            shutdown(QStringLiteral("shutdown while playback is active")),
        }};
}

} // namespace

QList<PlaybackProbeScenario> PlaybackProbeScenarioCatalog::create(
    const PlaybackProbeMediaSet& mediaSet)
{
    return {
        transportScenario(mediaSet),
        pausedSeekScenario(mediaSet),
        consecutiveSeekScenario(mediaSet),
        replacementScenario(mediaSet),
        endReasonScenario(mediaSet),
        loadErrorScenario(mediaSet),
        shutdownLoadingScenario(mediaSet),
        shutdownPlaybackScenario(mediaSet),
    };
}

} // namespace player::tools::playback_probe
