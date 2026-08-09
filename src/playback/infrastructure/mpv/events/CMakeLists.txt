target_sources(
    player_mpv_infrastructure
    PRIVATE
        mpv_event.h
        mpv_event_decoder.cpp
        mpv_event_decoder.h
        mpv_event_loop.cpp
        mpv_event_loop.h
        mpv_media_model_mapper.cpp
        mpv_media_model_mapper.h
        mpv_playback_event_mapper.cpp
        mpv_playback_event_mapper.h
        mpv_wakeup_bridge.cpp
        mpv_wakeup_bridge.h
)

add_subdirectory(media_model_mapping)

target_link_libraries(
    player_mpv_infrastructure
    PRIVATE
        player_playback_domain
)
