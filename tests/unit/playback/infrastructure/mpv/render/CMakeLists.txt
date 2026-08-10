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
    render_quick_fixture.h
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

add_executable(
    mpv_video_resize_dpi_tests
    mpv_video_resize_dpi_test.cpp
    render_quick_fixture.h
    render_test_fixture.h
    render_video_fixture.h
)

player_apply_compiler_options(mpv_video_resize_dpi_tests)
player_enable_compiler_warnings(mpv_video_resize_dpi_tests)
player_enable_sanitizers(mpv_video_resize_dpi_tests)
player_enable_static_analysis(mpv_video_resize_dpi_tests)

target_link_libraries(
    mpv_video_resize_dpi_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_video_resize_dpi_tests)

function(player_add_mpv_video_resize_dpi_test suffix scale_factor platform_smoke)
    set(test_name "mpv_video_resize_dpi_${suffix}")

    add_test(
        NAME "${test_name}"
        COMMAND mpv_video_resize_dpi_tests
    )

    set_tests_properties(
        "${test_name}"
        PROPERTIES
            ENVIRONMENT "QT_SCALE_FACTOR=${scale_factor};PLAYER_R4_06_FULLSCREEN_SMOKE=${platform_smoke};PLAYER_R4_06_MULTISCREEN_SMOKE=${platform_smoke}"
            RUN_SERIAL TRUE
            TIMEOUT 30
    )
endfunction()

player_add_mpv_video_resize_dpi_test(100 1.0 1)
player_add_mpv_video_resize_dpi_test(125 1.25 0)
player_add_mpv_video_resize_dpi_test(150 1.5 0)
player_add_mpv_video_resize_dpi_test(200 2.0 0)

add_executable(
    mpv_video_visibility_tests
    mpv_video_visibility_test.cpp
    render_pixel_capture_fixture.h
    render_test_fixture.h
    render_video_fixture.h
)

player_apply_compiler_options(mpv_video_visibility_tests)
player_enable_compiler_warnings(mpv_video_visibility_tests)
player_enable_sanitizers(mpv_video_visibility_tests)
player_enable_static_analysis(mpv_video_visibility_tests)

target_link_libraries(
    mpv_video_visibility_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_video_visibility_tests)

add_test(
    NAME mpv_video_visibility
    COMMAND mpv_video_visibility_tests
)

set_tests_properties(
    mpv_render_update_bridge
    mpv_video_renderer
    PROPERTIES
        RUN_SERIAL TRUE
        TIMEOUT 20
)

set_tests_properties(
    mpv_video_visibility
    PROPERTIES
        RUN_SERIAL TRUE
        TIMEOUT 45
)
