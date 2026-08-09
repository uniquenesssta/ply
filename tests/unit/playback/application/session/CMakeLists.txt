add_executable(
    playback_session_application_tests
    playback_session_test.cpp
    fixtures/session_test_media.cpp
    fixtures/session_test_media.h
)

player_apply_compiler_options(playback_session_application_tests)
player_enable_compiler_warnings(playback_session_application_tests)
player_enable_sanitizers(playback_session_application_tests)
player_enable_static_analysis(playback_session_application_tests)

target_link_libraries(
    playback_session_application_tests
    PRIVATE
        player_playback_application
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(playback_session_application_tests)

add_test(
    NAME playback_session
    COMMAND playback_session_application_tests
)
