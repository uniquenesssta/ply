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
        attributeCommandReply(event);
        return {};
    case MpvEventType::Shutdown:
    case MpvEventType::LogMessage:
    case MpvEventType::Unknown:
    case MpvEventType::DecodeFailure:
        return {};
    }

    return {};
}

std::optional<MediaGeneration>
MpvMediaGenerationAttributor::takePropertyRefreshGeneration() noexcept
{
    std::optional<MediaGeneration> generation = propertyRefreshGeneration_;
    propertyRefreshGeneration_.reset();
    return generation;
}

void MpvMediaGenerationAttributor::reset() noexcept
{
    pendingLoads_.clear();
    loadingEntries_.clear();
    entryGenerations_.clear();
    pendingStartEntries_.clear();
    unresolvedStartEntries_.clear();
    activePlaylistEntryId_ = 0;
    activeGeneration_ = {};
    propertyGeneration_ = {};
    propertyRefreshGeneration_.reset();
}

void MpvMediaGenerationAttributor::attributeCommandReply(const MpvEvent& event) noexcept
{
    if (event.replyUserdata == 0) {
        return;
    }

    const RequestId requestId{event.replyUserdata};
    const auto pending = std::find_if(
        pendingLoads_.begin(),
        pendingLoads_.end(),
        [requestId](const PendingLoad& load) {
            return load.requestId == requestId;
        });
    if (pending == pendingLoads_.end()) {
        return;
    }

    const MediaGeneration generation = pending->generation;
    pendingLoads_.erase(pending);

    if (!event.error.isSuccess()) {
        if (pendingLoads_.empty()) {
            unresolvedStartEntries_.clear();
        }
        return;
    }

    const auto* reply = std::get_if<MpvCommandReplyData>(&event.payload);
    if (reply == nullptr || !reply->playlistEntryId.has_value()
        || *reply->playlistEntryId <= 0) {
        if (pendingLoads_.empty()) {
            unresolvedStartEntries_.clear();
        }
        return;
    }

    const qint64 playlistEntryId = *reply->playlistEntryId;
    const bool startAlreadyObserved = unresolvedStartEntries_.erase(playlistEntryId) > 0;

    if (activeGeneration_.isValid()
        && generation.value() < activeGeneration_.value()
        && !startAlreadyObserved) {
        if (pendingLoads_.empty()) {
            unresolvedStartEntries_.clear();
        }
        return;
    }

    entryGenerations_[playlistEntryId] = generation;
    if (startAlreadyObserved) {
        (void)activateStartFile(playlistEntryId, generation);
    } else {
        pendingStartEntries_.insert(playlistEntryId);
    }

    if (pendingLoads_.empty()) {
        unresolvedStartEntries_.clear();
    }
}

MediaGeneration MpvMediaGenerationAttributor::attributeStartFile(
    const MpvEvent& event) noexcept
{
    const auto* startFile = std::get_if<MpvStartFileData>(&event.payload);
    if (startFile == nullptr || startFile->playlistEntryId <= 0) {
        return {};
    }

    const qint64 playlistEntryId = startFile->playlistEntryId;
    const auto generation = entryGenerations_.find(playlistEntryId);
    if (generation == entryGenerations_.end()) {
        if (!pendingLoads_.empty()) {
            unresolvedStartEntries_.insert(playlistEntryId);
        }
        return {};
    }

    return activateStartFile(playlistEntryId, generation->second);
}

MediaGeneration MpvMediaGenerationAttributor::activateStartFile(
    qint64 playlistEntryId,
    MediaGeneration generation) noexcept
{
    pendingStartEntries_.erase(playlistEntryId);
    discardSupersededPendingStarts(generation);

    const MediaGeneration previousActiveGeneration = activeGeneration_;
    if (previousActiveGeneration.isValid()
        && generation.value() < previousActiveGeneration.value()) {
        return generation;
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
            propertyRefreshGeneration_ = generation->second;
        }
        return generation->second;
    }

    return {};
}

MediaGeneration MpvMediaGenerationAttributor::attributeEndFile(
    const MpvEvent& event) noexcept
{
    const auto* endFile = std::get_if<MpvEndFileData>(&event.payload);
    if (endFile == nullptr || endFile->playlistEntryId <= 0) {
        return {};
    }

    const qint64 playlistEntryId = endFile->playlistEntryId;
    unresolvedStartEntries_.erase(playlistEntryId);
    pendingStartEntries_.erase(playlistEntryId);

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
            if (!hasReplacementPending() && propertyGeneration_ == generation) {
                propertyGeneration_ = {};
            }
        }
    }

    entryGenerations_.erase(playlistEntryId);
    return generation;
}

void MpvMediaGenerationAttributor::discardSupersededPendingStarts(
    MediaGeneration generation) noexcept
{
    for (auto entry = pendingStartEntries_.begin(); entry != pendingStartEntries_.end();) {
        const auto mapped = entryGenerations_.find(*entry);
        if (mapped == entryGenerations_.end()
            || mapped->second.value() < generation.value()) {
            if (mapped != entryGenerations_.end()) {
                entryGenerations_.erase(mapped);
            }
            entry = pendingStartEntries_.erase(entry);
            continue;
        }
        ++entry;
    }
}

bool MpvMediaGenerationAttributor::hasReplacementPending() const noexcept
{
    return !pendingLoads_.empty()
        || !pendingStartEntries_.empty()
        || !unresolvedStartEntries_.empty();
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
