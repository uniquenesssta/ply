add_executable(
    player_fullscreen_controls_tests
    fullscreen_controls_test.cpp
)

player_apply_compiler_options(player_fullscreen_controls_tests)
player_enable_compiler_warnings(player_fullscreen_controls_tests)
player_enable_sanitizers(player_fullscreen_controls_tests)
player_enable_static_analysis(player_fullscreen_controls_tests)

target_compile_definitions(
    player_fullscreen_controls_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_fullscreen_controls_tests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_fullscreen_controls
    COMMAND player_fullscreen_controls_tests
)
