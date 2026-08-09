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

target_link_libraries(
    player_mpv_infrastructure
    PRIVATE
        Qt6::Gui
)
