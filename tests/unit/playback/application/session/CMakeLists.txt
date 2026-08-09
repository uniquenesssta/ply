add_executable(
    playback_session_application_tests
    playback_session_test.cpp
    fixtures/session_test_media.cpp
    fixtures/session_test_media.h
    support/playback_session_test_harness.cpp
    support/playback_session_test_harness.h
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

add_executable(
    playback_shutdown_application_tests
    playback_shutdown_test.cpp
    fixtures/session_test_media.cpp
    fixtures/session_test_media.h
)

player_apply_compiler_options(playback_shutdown_application_tests)
player_enable_compiler_warnings(playback_shutdown_application_tests)
player_enable_sanitizers(playback_shutdown_application_tests)
player_enable_static_analysis(playback_shutdown_application_tests)

target_link_libraries(
    playback_shutdown_application_tests
    PRIVATE
        player_playback_application
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(playback_shutdown_application_tests)

add_test(
    NAME playback_shutdown
    COMMAND playback_shutdown_application_tests
)
set_tests_properties(playback_shutdown PROPERTIES TIMEOUT 120)

add_executable(
    playback_media_generation_application_tests
    media_generation_gate_test.cpp
)

player_apply_compiler_options(playback_media_generation_application_tests)
player_enable_compiler_warnings(playback_media_generation_application_tests)
player_enable_sanitizers(playback_media_generation_application_tests)
player_enable_static_analysis(playback_media_generation_application_tests)

target_link_libraries(
    playback_media_generation_application_tests
    PRIVATE
        player_playback_application
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(playback_media_generation_application_tests)

add_test(
    NAME playback_media_generation
    COMMAND playback_media_generation_application_tests
)
