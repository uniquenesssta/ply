add_library(player_presentation_status STATIC)

target_sources(
    player_presentation_status
    PRIVATE
        player_status_selector.cpp
        player_status_view_model.cpp
    PUBLIC
        player_status_selector.h
        player_status_view_model.h
)

player_apply_compiler_options(player_presentation_status)
player_enable_compiler_warnings(player_presentation_status)
player_enable_sanitizers(player_presentation_status)
player_enable_static_analysis(player_presentation_status)

target_include_directories(
    player_presentation_status
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
    player_presentation_status
    PUBLIC
        player_playback_domain
        Qt6::Core
)
