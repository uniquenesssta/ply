#pragma once

#include "playback/infrastructure/mpv/events/mpv_event.h"

#include <QList>
#include <QString>
#include <QtGlobal>

namespace player::tools::playback_probe {

class PlaybackProbeTrace final
{
public:
    void reset(const QString& scenarioId);
    void recordCommand(quint64 requestId, const QString& label);
    void recordMarker(const QString& marker);
    void recordEvent(const player::playback::mpv::MpvEvent& event);
    void dump() const;

private:
    void append(QString entry);

    static constexpr qsizetype kMaximumEntries = 512;

    QString scenarioId_;
    QList<QString> entries_;
    bool truncated_ = false;
};

} // namespace player::tools::playback_probe
