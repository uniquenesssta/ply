add_executable(
    render_probe
    main.cpp
    render_probe_environment.cpp
    render_probe_environment.h
    render_probe_metrics.cpp
    render_probe_metrics.h
    render_probe_options.cpp
    render_probe_options.h
    render_probe_report.cpp
    render_probe_report.h
    render_probe_runner.cpp
    render_probe_runner.h
)

target_include_directories(
    render_probe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

player_apply_compiler_options(render_probe)
player_enable_compiler_warnings(render_probe)
player_enable_sanitizers(render_probe)
player_enable_static_analysis(render_probe)

target_link_libraries(
    render_probe
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
)

set_target_properties(
    render_probe
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

player_stage_libmpv_runtime(render_probe)

if(PLAYER_BUILD_TESTS)
    add_test(
        NAME render_probe_help
        COMMAND render_probe --help
    )
    set_tests_properties(
        render_probe_help
        PROPERTIES
            TIMEOUT 10
    )
endif()
