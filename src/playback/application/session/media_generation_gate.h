#pragma once

#include "playback/domain/events/playback_event.h"
#include "playback/domain/state/media_generation.h"

#include <QtGlobal>

namespace player::playback::application {

struct MediaGenerationGateDiagnostics final
{
    quint64 missingGenerationEventCount = 0;
    quint64 staleGenerationEventCount = 0;
};

class MediaGenerationGate final
{
public:
    void activate(player::playback::domain::MediaGeneration generation) noexcept;
    void reset() noexcept;

    [[nodiscard]] bool accepts(
        const player::playback::domain::PlaybackEvent& event) noexcept;

    [[nodiscard]] player::playback::domain::MediaGeneration currentGeneration() const noexcept;
    [[nodiscard]] const MediaGenerationGateDiagnostics& diagnostics() const noexcept;

    [[nodiscard]] static bool isMediaScoped(
        const player::playback::domain::PlaybackEvent& event) noexcept;

private:
    player::playback::domain::MediaGeneration currentGeneration_;
    MediaGenerationGateDiagnostics diagnostics_;
};

} // namespace player::playback::application
