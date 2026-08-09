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
