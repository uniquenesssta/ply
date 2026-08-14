add_executable(
    player_screen_structure_tests
    player_screen_structure_test.cpp
)

player_apply_compiler_options(player_screen_structure_tests)
player_enable_compiler_warnings(player_screen_structure_tests)
player_enable_sanitizers(player_screen_structure_tests)
player_enable_static_analysis(player_screen_structure_tests)

add_dependencies(
    player_screen_structure_tests
    player_qml_lint
)

target_compile_definitions(
    player_screen_structure_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_screen_structure_tests
    PRIVATE
        Qt6::Test
)

add_test(
    NAME player_screen_structure
    COMMAND player_screen_structure_tests
)
