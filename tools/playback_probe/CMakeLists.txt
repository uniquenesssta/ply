add_executable(
    playback_probe
    main.cpp
    playback_probe_matrix_runner.cpp
    playback_probe_matrix_runner.h
    playback_probe_runner.cpp
    playback_probe_runner.h
    fixtures/playback_probe_media_set.cpp
    fixtures/playback_probe_media_set.h
    runtime/playback_probe_runtime.cpp
    runtime/playback_probe_runtime.h
    scenarios/playback_probe_scenario.h
    scenarios/playback_probe_scenario_catalog.cpp
    scenarios/playback_probe_scenario_catalog.h
    scenarios/playback_probe_scenario_runner.cpp
    scenarios/playback_probe_scenario_runner.h
    trace/playback_probe_trace.cpp
    trace/playback_probe_trace.h
)

player_apply_compiler_options(playback_probe)
player_enable_compiler_warnings(playback_probe)
player_enable_sanitizers(playback_probe)
player_enable_static_analysis(playback_probe)

target_link_libraries(
    playback_probe
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
)

player_stage_libmpv_runtime(playback_probe)

add_custom_command(
    TARGET playback_probe
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Qt6::Core>"
        "$<TARGET_FILE_DIR:playback_probe>"
    VERBATIM
)

if(PLAYER_BUILD_TESTS)
    add_test(
        NAME playback_probe_matrix
        COMMAND playback_probe --matrix
    )
    set_tests_properties(
        playback_probe_matrix
        PROPERTIES
            TIMEOUT 90
    )
endif()
