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

qt_wrap_cpp(
    player_mpv_infrastructure
    mpv_render_update_bridge.h
)

target_link_libraries(
    player_mpv_infrastructure
    PRIVATE
        Qt6::Gui
)
