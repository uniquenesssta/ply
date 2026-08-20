add_library(
    player_tracks_presentation
    STATIC
        chapter_model.cpp
        chapter_model.h
        track_list_model.cpp
        track_list_model.h
)

player_apply_compiler_options(player_tracks_presentation)
player_enable_compiler_warnings(player_tracks_presentation)
player_enable_sanitizers(player_tracks_presentation)
player_enable_static_analysis(player_tracks_presentation)

target_include_directories(
    player_tracks_presentation
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    player_tracks_presentation
    PUBLIC
        player_playback_domain
        Qt6::Core
)
