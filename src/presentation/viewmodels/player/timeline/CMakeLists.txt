add_library(player_presentation_timeline STATIC)

target_sources(
    player_presentation_timeline
    PRIVATE
        timeline_scrub_session.cpp
        timeline_seek_projection.cpp
        timeline_relative_seek_coalescer.cpp
        player_timeline_view_model.cpp
    PUBLIC
        timeline_scrub_session.h
        timeline_seek_projection.h
        timeline_relative_seek_coalescer.h
        player_timeline_view_model.h
)

player_apply_compiler_options(player_presentation_timeline)
player_enable_compiler_warnings(player_presentation_timeline)
player_enable_sanitizers(player_presentation_timeline)
player_enable_static_analysis(player_presentation_timeline)

target_include_directories(
    player_presentation_timeline
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
    player_presentation_timeline
    PUBLIC
        player_playback_domain
        Qt6::Core
)
