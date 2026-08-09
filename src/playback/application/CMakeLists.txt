add_library(player_playback_application STATIC)

add_subdirectory(requests)
add_subdirectory(session)
add_subdirectory(command_bus)

player_apply_compiler_options(player_playback_application)
player_enable_compiler_warnings(player_playback_application)
player_enable_sanitizers(player_playback_application)
player_enable_static_analysis(player_playback_application)

target_include_directories(
    player_playback_application
    PUBLIC
        ../..
)

target_link_libraries(
    player_playback_application
    PUBLIC
        player_playback_domain
        Qt6::Core
    PRIVATE
        player_mpv_infrastructure
)
