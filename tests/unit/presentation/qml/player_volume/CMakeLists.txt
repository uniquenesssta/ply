add_executable(
    player_volume_controls_tests
    volume_controls_test.cpp
)

player_apply_compiler_options(player_volume_controls_tests)
player_enable_compiler_warnings(player_volume_controls_tests)
player_enable_sanitizers(player_volume_controls_tests)
player_enable_static_analysis(player_volume_controls_tests)

target_compile_definitions(
    player_volume_controls_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_volume_controls_tests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_volume_controls
    COMMAND player_volume_controls_tests
)
