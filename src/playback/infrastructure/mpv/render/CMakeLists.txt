target_sources(
    player_mpv_infrastructure
    PRIVATE
        mpv_render_context.cpp
        mpv_render_context.h
        mpv_render_parameters.cpp
        mpv_render_parameters.h
        mpv_render_shutdown_coordinator.cpp
        mpv_render_shutdown_coordinator.h
        mpv_render_update_bridge.cpp
        mpv_render_update_bridge.h
        mpv_render_visibility_policy.cpp
        mpv_render_visibility_policy.h
        mpv_video_item.cpp
        mpv_video_item.h
        mpv_video_renderer.cpp
        mpv_video_renderer.h
        opengl_proc_resolver.cpp
        opengl_proc_resolver.h
)

set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/mpv_render_update_bridge.h"
    TARGET_DIRECTORY player_mpv_infrastructure
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
    TARGET_DIRECTORY player_mpv_infrastructure
    PROPERTIES
        GENERATED TRUE
        SKIP_AUTOMOC ON
)

add_custom_target(
    player_mpv_render_update_bridge_moc
    DEPENDS
        "${mpv_render_update_bridge_moc_source}"
)

add_dependencies(
    player_mpv_infrastructure
    player_mpv_render_update_bridge_moc
)

target_sources(
    player_mpv_infrastructure
    PRIVATE
        "${mpv_render_update_bridge_moc_source}"
)

set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/mpv_video_item.h"
    TARGET_DIRECTORY player_mpv_infrastructure
    PROPERTIES
        SKIP_AUTOMOC ON
)

set(
    mpv_video_item_moc_source
    "${CMAKE_CURRENT_BINARY_DIR}/moc_mpv_video_item.cpp"
)

qt_generate_moc(
    "${CMAKE_CURRENT_SOURCE_DIR}/mpv_video_item.h"
    "${mpv_video_item_moc_source}"
    TARGET player_mpv_infrastructure
)

set_source_files_properties(
    "${mpv_video_item_moc_source}"
    TARGET_DIRECTORY player_mpv_infrastructure
    PROPERTIES
        GENERATED TRUE
        SKIP_AUTOMOC ON
)

add_custom_target(
    player_mpv_video_item_moc
    DEPENDS
        "${mpv_video_item_moc_source}"
)

add_dependencies(
    player_mpv_infrastructure
    player_mpv_video_item_moc
)

target_sources(
    player_mpv_infrastructure
    PRIVATE
        "${mpv_video_item_moc_source}"
)

target_link_libraries(
    player_mpv_infrastructure
    PUBLIC
        Qt6::Quick
    PRIVATE
        Qt6::Gui
)
