#include "mpv_media_generation_attributor.h"

#include <algorithm>
#include <variant>

namespace player::playback::application {

using player::ids::RequestId;
using player::playback::domain::MediaGeneration;
using namespace player::playback::mpv;

void MpvMediaGenerationAttributor::noteLoadSubmission(
    RequestId requestId,
    MediaGeneration generation)
{
    if (!requestId.isValid() || !generation.isValid()) {
        return;
    }

    pendingLoads_.push_back(PendingLoad{requestId, generation});
}

void MpvMediaGenerationAttributor::cancelLoadSubmission(RequestId requestId) noexcept
{
    const auto match = std::find_if(
        pendingLoads_.begin(),
        pendingLoads_.end(),
        [requestId](const PendingLoad& pending) {
            return pending.requestId == requestId;
        });
    if (match != pendingLoads_.end()) {
        pendingLoads_.erase(match);
    }
}

MediaGeneration MpvMediaGenerationAttributor::attribute(const MpvEvent& event) noexcept
{
    switch (event.type) {
    case MpvEventType::StartFile:
        return attributeStartFile(event);
    case MpvEventType::FileLoaded:
        return attributeFileLoaded();
    case MpvEventType::EndFile:
        return attributeEndFile(event);
    case MpvEventType::PropertyChange:
        return propertyGeneration_;
    case MpvEventType::CommandReply:
        if (!event.error.isSuccess() && event.replyUserdata != 0) {
            cancelLoadSubmission(RequestId{event.replyUserdata});
        }
        return {};
    case MpvEventType::Shutdown:
    case MpvEventType::LogMessage:
    case MpvEventType::Unknown:
    case MpvEventType::DecodeFailure:
        return {};
    }

    return {};
}

void MpvMediaGenerationAttributor::reset() noexcept
{
    pendingLoads_.clear();
    loadingEntries_.clear();
    entryGenerations_.clear();
    activePlaylistEntryId_ = 0;
    activeGeneration_ = {};
    propertyGeneration_ = {};
}

MediaGeneration MpvMediaGenerationAttributor::attributeStartFile(
    const MpvEvent& event) noexcept
{
    const auto* startFile = std::get_if<MpvStartFileData>(&event.payload);
    if (startFile == nullptr || startFile->playlistEntryId <= 0) {
        return {};
    }

    const qint64 playlistEntryId = startFile->playlistEntryId;
    const MediaGeneration previousActiveGeneration = activeGeneration_;
    MediaGeneration generation;

    if (const auto existing = entryGenerations_.find(playlistEntryId);
        existing != entryGenerations_.end()) {
        generation = existing->second;
    } else if (!pendingLoads_.empty()) {
        generation = pendingLoads_.front().generation;
        pendingLoads_.pop_front();
        entryGenerations_.emplace(playlistEntryId, generation);
    } else if (activeGeneration_.isValid()) {
        generation = activeGeneration_;
        entryGenerations_.emplace(playlistEntryId, generation);
    }

    if (!generation.isValid()) {
        return {};
    }

    activePlaylistEntryId_ = playlistEntryId;
    activeGeneration_ = generation;

    if (!propertyGeneration_.isValid() && !previousActiveGeneration.isValid()) {
        propertyGeneration_ = generation;
    } else if (previousActiveGeneration == generation) {
        propertyGeneration_ = generation;
    }

    if (std::find(loadingEntries_.begin(), loadingEntries_.end(), playlistEntryId)
        == loadingEntries_.end()) {
        loadingEntries_.push_back(playlistEntryId);
    }
    return generation;
}

MediaGeneration MpvMediaGenerationAttributor::attributeFileLoaded() noexcept
{
    while (!loadingEntries_.empty()) {
        const qint64 playlistEntryId = loadingEntries_.front();
        loadingEntries_.pop_front();
        const auto generation = entryGenerations_.find(playlistEntryId);
        if (generation == entryGenerations_.end()) {
            continue;
        }

        if (playlistEntryId == activePlaylistEntryId_) {
            propertyGeneration_ = generation->second;
        }
        return generation->second;
    }

    return activeGeneration_;
}

MediaGeneration MpvMediaGenerationAttributor::attributeEndFile(
    const MpvEvent& event) noexcept
{
    const auto* endFile = std::get_if<MpvEndFileData>(&event.payload);
    if (endFile == nullptr || endFile->playlistEntryId <= 0) {
        return {};
    }

    const qint64 playlistEntryId = endFile->playlistEntryId;
    const auto generationEntry = entryGenerations_.find(playlistEntryId);
    if (generationEntry == entryGenerations_.end()) {
        return {};
    }

    const MediaGeneration generation = generationEntry->second;
    removeLoadingEntry(playlistEntryId);

    if (endFile->reason == MpvEndFileReason::Redirect
        && endFile->playlistInsertId > 0
        && endFile->playlistInsertNumEntries > 0) {
        for (int offset = 0; offset < endFile->playlistInsertNumEntries; ++offset) {
            entryGenerations_[endFile->playlistInsertId + offset] = generation;
        }
    }

    if (activePlaylistEntryId_ == playlistEntryId) {
        activePlaylistEntryId_ = 0;
        if (endFile->reason != MpvEndFileReason::Redirect) {
            activeGeneration_ = {};
            const bool replacementStillPending = !pendingLoads_.empty();
            if (!replacementStillPending && propertyGeneration_ == generation) {
                propertyGeneration_ = {};
            }
        }
    }

    entryGenerations_.erase(playlistEntryId);
    return generation;
}

void MpvMediaGenerationAttributor::removeLoadingEntry(qint64 playlistEntryId) noexcept
{
    const auto entry = std::find(
        loadingEntries_.begin(),
        loadingEntries_.end(),
        playlistEntryId);
    if (entry != loadingEntries_.end()) {
        loadingEntries_.erase(entry);
    }
}

} // namespace player::playback::application
