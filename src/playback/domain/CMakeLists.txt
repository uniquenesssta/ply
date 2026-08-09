add_library(player_playback_domain STATIC)

add_subdirectory(errors)
add_subdirectory(models)
add_subdirectory(commands)
add_subdirectory(events)
add_subdirectory(state)

player_apply_compiler_options(player_playback_domain)
player_enable_compiler_warnings(player_playback_domain)
player_enable_sanitizers(player_playback_domain)
player_enable_static_analysis(player_playback_domain)

target_include_directories(
    player_playback_domain
    PUBLIC
        ../..
)

target_link_libraries(
    player_playback_domain
    PUBLIC
        player_foundation
        Qt6::Core
)
