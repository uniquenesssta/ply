add_executable(
    player_timeline_controls_tests
    timeline_controls_test.cpp
)

player_apply_compiler_options(player_timeline_controls_tests)
player_enable_compiler_warnings(player_timeline_controls_tests)
player_enable_sanitizers(player_timeline_controls_tests)
player_enable_static_analysis(player_timeline_controls_tests)

target_compile_definitions(
    player_timeline_controls_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_timeline_controls_tests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_timeline_controls
    COMMAND player_timeline_controls_tests
)
