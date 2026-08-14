add_library(player_presentation_transport STATIC)

target_sources(
    player_presentation_transport
    PRIVATE
        player_transport_view_model.cpp
    PUBLIC
        player_transport_view_model.h
)

player_apply_compiler_options(player_presentation_transport)
player_enable_compiler_warnings(player_presentation_transport)
player_enable_sanitizers(player_presentation_transport)
player_enable_static_analysis(player_presentation_transport)

target_include_directories(
    player_presentation_transport
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
    player_presentation_transport
    PUBLIC
        player_playback_domain
        Qt6::Core
)
