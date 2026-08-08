add_executable(
    playback_probe
    main.cpp
    playback_probe_runner.cpp
    playback_probe_runner.h
    runtime/playback_probe_runtime.cpp
    runtime/playback_probe_runtime.h
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
