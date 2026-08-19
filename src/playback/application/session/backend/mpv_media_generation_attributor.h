#pragma once

#include "foundation/ids/request_id.h"
#include "playback/domain/state/media_generation.h"
#include "playback/infrastructure/mpv/events/mpv_event.h"

#include <QtGlobal>

#include <deque>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace player::playback::application {

class MpvMediaGenerationAttributor final
{
public:
    void noteLoadSubmission(
        player::ids::RequestId requestId,
        player::playback::domain::MediaGeneration generation);
    void cancelLoadSubmission(player::ids::RequestId requestId) noexcept;

    [[nodiscard]] player::playback::domain::MediaGeneration attribute(
        const player::playback::mpv::MpvEvent& event) noexcept;
    [[nodiscard]] std::optional<player::playback::domain::MediaGeneration>
    takePropertyRefreshGeneration() noexcept;

    void reset() noexcept;

private:
    struct PendingLoad final
    {
        player::ids::RequestId requestId;
        player::playback::domain::MediaGeneration generation;
    };

    void attributeCommandReply(
        const player::playback::mpv::MpvEvent& event) noexcept;
    [[nodiscard]] player::playback::domain::MediaGeneration attributeStartFile(
        const player::playback::mpv::MpvEvent& event) noexcept;
    [[nodiscard]] player::playback::domain::MediaGeneration activateStartFile(
        qint64 playlistEntryId,
        player::playback::domain::MediaGeneration generation) noexcept;
    [[nodiscard]] player::playback::domain::MediaGeneration attributeFileLoaded() noexcept;
    [[nodiscard]] player::playback::domain::MediaGeneration attributeEndFile(
        const player::playback::mpv::MpvEvent& event) noexcept;

    void discardSupersededPendingStarts(
        player::playback::domain::MediaGeneration generation) noexcept;
    [[nodiscard]] bool hasReplacementPending() const noexcept;
    void removeLoadingEntry(qint64 playlistEntryId) noexcept;

    std::deque<PendingLoad> pendingLoads_;
    std::deque<qint64> loadingEntries_;
    std::unordered_map<qint64, player::playback::domain::MediaGeneration> entryGenerations_;
    std::unordered_set<qint64> pendingStartEntries_;
    std::unordered_set<qint64> unresolvedStartEntries_;
    qint64 activePlaylistEntryId_ = 0;
    player::playback::domain::MediaGeneration activeGeneration_;
    player::playback::domain::MediaGeneration propertyGeneration_;
    std::optional<player::playback::domain::MediaGeneration> propertyRefreshGeneration_;
};

} // namespace player::playback::application
