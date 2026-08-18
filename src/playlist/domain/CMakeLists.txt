add_library(
    player_playlist_domain
    STATIC
        playlist.cpp
        playlist.h
        playlist_entry.cpp
        playlist_entry.h
        playlist_entry_id.h
        playlist_navigation.cpp
        playlist_navigation.h
        playlist_repeat_mode.h
        playlist_shuffle_state.cpp
        playlist_shuffle_state.h
        playlist_snapshot.cpp
        playlist_snapshot.h
)

player_apply_compiler_options(player_playlist_domain)
player_enable_compiler_warnings(player_playlist_domain)
player_enable_sanitizers(player_playlist_domain)
player_enable_static_analysis(player_playlist_domain)

target_include_directories(
    player_playlist_domain
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    player_playlist_domain
    PUBLIC
        player_media_domain
        Qt6::Core
)
