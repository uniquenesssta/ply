target_sources(
    player_mpv_infrastructure
    PRIVATE
        mpv_render_context.cpp
        mpv_render_context.h
        mpv_render_update_bridge.cpp
        mpv_render_update_bridge.h
        opengl_proc_resolver.cpp
        opengl_proc_resolver.h
)

set_source_files_properties(
    mpv_render_update_bridge.h
    PROPERTIES
        SKIP_AUTOMOC ON
)

set(
    mpv_render_update_bridge_moc_source
    "${CMAKE_CURRENT_BINARY_DIR}/moc_mpv_render_update_bridge.cpp"
)

qt_generate_moc(
    "${CMAKE_CURRENT_SOURCE_DIR}/mpv_render_update_bridge.h"
    "${mpv_render_update_bridge_moc_source}"
    TARGET player_mpv_infrastructure
)

set_source_files_properties(
    "${mpv_render_update_bridge_moc_source}"
    PROPERTIES
        GENERATED TRUE
        SKIP_AUTOMOC ON
)

target_sources(
    player_mpv_infrastructure
    PRIVATE
        "${mpv_render_update_bridge_moc_source}"
)

target_link_libraries(
    player_mpv_infrastructure
    PRIVATE
        Qt6::Gui
)
