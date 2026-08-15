add_library(player_presentation_hud STATIC)

target_sources(
    player_presentation_hud
    PRIVATE
        hud_message_queue.cpp
    PUBLIC
        hud_message_queue.h
)

player_apply_compiler_options(player_presentation_hud)
player_enable_compiler_warnings(player_presentation_hud)
player_enable_sanitizers(player_presentation_hud)
player_enable_static_analysis(player_presentation_hud)

target_include_directories(
    player_presentation_hud
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
    player_presentation_hud
    PUBLIC
        Qt6::Core
)
