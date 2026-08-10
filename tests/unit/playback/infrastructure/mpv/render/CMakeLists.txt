add_executable(
    opengl_proc_resolver_tests
    opengl_proc_resolver_test.cpp
)

player_apply_compiler_options(opengl_proc_resolver_tests)
player_enable_compiler_warnings(opengl_proc_resolver_tests)
player_enable_sanitizers(opengl_proc_resolver_tests)
player_enable_static_analysis(opengl_proc_resolver_tests)

target_link_libraries(
    opengl_proc_resolver_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

player_stage_libmpv_runtime(opengl_proc_resolver_tests)

add_test(
    NAME opengl_proc_resolver
    COMMAND opengl_proc_resolver_tests
)

add_executable(
    mpv_render_context_tests
    mpv_render_context_test.cpp
    render_test_fixture.h
)

player_apply_compiler_options(mpv_render_context_tests)
player_enable_compiler_warnings(mpv_render_context_tests)
player_enable_sanitizers(mpv_render_context_tests)
player_enable_static_analysis(mpv_render_context_tests)

target_link_libraries(
    mpv_render_context_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_render_context_tests)

add_test(
    NAME mpv_render_context
    COMMAND mpv_render_context_tests
)

add_executable(
    mpv_render_update_bridge_tests
    mpv_render_update_bridge_test.cpp
    render_test_fixture.h
    render_video_fixture.h
)

player_apply_compiler_options(mpv_render_update_bridge_tests)
player_enable_compiler_warnings(mpv_render_update_bridge_tests)
player_enable_sanitizers(mpv_render_update_bridge_tests)
player_enable_static_analysis(mpv_render_update_bridge_tests)

target_link_libraries(
    mpv_render_update_bridge_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_render_update_bridge_tests)

add_test(
    NAME mpv_render_update_bridge
    COMMAND mpv_render_update_bridge_tests
)

add_executable(
    mpv_video_item_tests
    mpv_video_item_test.cpp
)

player_apply_compiler_options(mpv_video_item_tests)
player_enable_compiler_warnings(mpv_video_item_tests)
player_enable_sanitizers(mpv_video_item_tests)
player_enable_static_analysis(mpv_video_item_tests)

target_link_libraries(
    mpv_video_item_tests
    PRIVATE
        player_mpv_infrastructure
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_video_item_tests)

add_test(
    NAME mpv_video_item
    COMMAND mpv_video_item_tests
)

add_executable(
    mpv_video_renderer_tests
    mpv_video_renderer_test.cpp
    render_test_fixture.h
    render_video_fixture.h
)

player_apply_compiler_options(mpv_video_renderer_tests)
player_enable_compiler_warnings(mpv_video_renderer_tests)
player_enable_sanitizers(mpv_video_renderer_tests)
player_enable_static_analysis(mpv_video_renderer_tests)

target_link_libraries(
    mpv_video_renderer_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_video_renderer_tests)

add_test(
    NAME mpv_video_renderer
    COMMAND mpv_video_renderer_tests
)

set_tests_properties(
    mpv_render_update_bridge
    mpv_video_renderer
    PROPERTIES
        TIMEOUT 20
)
