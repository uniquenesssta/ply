add_executable(
    playback_request_tracker_application_tests
    request_tracker_test.cpp
)

player_apply_compiler_options(playback_request_tracker_application_tests)
player_enable_compiler_warnings(playback_request_tracker_application_tests)
player_enable_sanitizers(playback_request_tracker_application_tests)
player_enable_static_analysis(playback_request_tracker_application_tests)

target_link_libraries(
    playback_request_tracker_application_tests
    PRIVATE
        player_playback_application
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(playback_request_tracker_application_tests)

add_test(
    NAME playback_request_tracker
    COMMAND playback_request_tracker_application_tests
)
