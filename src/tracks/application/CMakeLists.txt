add_library(
    player_tracks_application
    STATIC
        track_selection_controller.cpp
        track_selection_controller.h
)

player_apply_compiler_options(player_tracks_application)
player_enable_compiler_warnings(player_tracks_application)
player_enable_sanitizers(player_tracks_application)
player_enable_static_analysis(player_tracks_application)

target_include_directories(
    player_tracks_application
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    player_tracks_application
    PUBLIC
        player_playback_domain
        Qt6::Core
)
