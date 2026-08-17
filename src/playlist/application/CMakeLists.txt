add_library(
    player_playlist_application
    STATIC
        playlist_auto_advance.cpp
        playlist_auto_advance.h
        playlist_controller.cpp
        playlist_controller.h
        playlist_mutation.cpp
        playlist_mutation.h
)

player_apply_compiler_options(player_playlist_application)
player_enable_compiler_warnings(player_playlist_application)
player_enable_sanitizers(player_playlist_application)
player_enable_static_analysis(player_playlist_application)

target_include_directories(
    player_playlist_application
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    player_playlist_application
    PUBLIC
        player_playlist_domain
        Qt6::Core
)
