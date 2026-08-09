target_sources(
    player_playback_application
    PRIVATE
        request_tracker.cpp
    PUBLIC
        playback_request.h
        request_tracker.h
)
