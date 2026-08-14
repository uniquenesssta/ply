add_library(player_presentation_volume STATIC)

target_sources(
    player_presentation_volume
    PRIVATE
        player_volume_view_model.cpp
    PUBLIC
        player_volume_view_model.h
)

player_apply_compiler_options(player_presentation_volume)
player_enable_compiler_warnings(player_presentation_volume)
player_enable_sanitizers(player_presentation_volume)
player_enable_static_analysis(player_presentation_volume)

target_include_directories(
    player_presentation_volume
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
    player_presentation_volume
    PUBLIC
        player_playback_domain
        Qt6::Core
)
