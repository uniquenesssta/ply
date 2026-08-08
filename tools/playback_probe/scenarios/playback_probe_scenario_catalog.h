#pragma once

#include "playback_probe_scenario.h"

#include <QList>

namespace player::tools::playback_probe {

class PlaybackProbeMediaSet;

class PlaybackProbeScenarioCatalog final
{
public:
    [[nodiscard]] static QList<PlaybackProbeScenario> create(
        const PlaybackProbeMediaSet& mediaSet);
};

} // namespace player::tools::playback_probe
