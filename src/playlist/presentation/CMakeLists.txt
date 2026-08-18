add_library(
    player_playlist_presentation
    STATIC
        playlist_entry_playback_state.cpp
        playlist_entry_playback_state.h
        playlist_list_model.cpp
        playlist_list_model.h
)

player_apply_compiler_options(player_playlist_presentation)
player_enable_compiler_warnings(player_playlist_presentation)
player_enable_sanitizers(player_playlist_presentation)
player_enable_static_analysis(player_playlist_presentation)

target_include_directories(
    player_playlist_presentation
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    player_playlist_presentation
    PUBLIC
        player_playlist_application
        player_playback_domain
        Qt6::Core
)
