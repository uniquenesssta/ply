add_executable(
    media_open_coordinator_tests
    media_open_coordinator_test.cpp
)

player_apply_compiler_options(media_open_coordinator_tests)
player_enable_compiler_warnings(media_open_coordinator_tests)
player_enable_sanitizers(media_open_coordinator_tests)
player_enable_static_analysis(media_open_coordinator_tests)

target_link_libraries(
    media_open_coordinator_tests
    PRIVATE
        player_media_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME media_open_coordinator
    COMMAND media_open_coordinator_tests
)
