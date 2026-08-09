add_executable(
    playback_state_publisher_application_tests
    state_publisher_test.cpp
)

player_apply_compiler_options(playback_state_publisher_application_tests)
player_enable_compiler_warnings(playback_state_publisher_application_tests)
player_enable_sanitizers(playback_state_publisher_application_tests)
player_enable_static_analysis(playback_state_publisher_application_tests)

target_link_libraries(
    playback_state_publisher_application_tests
    PRIVATE
        player_playback_application
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(playback_state_publisher_application_tests)

add_test(
    NAME playback_state_publisher
    COMMAND playback_state_publisher_application_tests
)
