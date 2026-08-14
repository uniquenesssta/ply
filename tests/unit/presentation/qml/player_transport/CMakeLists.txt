add_executable(
    player_transport_control_tests
    transport_controls_test.cpp
)

player_apply_compiler_options(player_transport_control_tests)
player_enable_compiler_warnings(player_transport_control_tests)
player_enable_sanitizers(player_transport_control_tests)
player_enable_static_analysis(player_transport_control_tests)

add_dependencies(
    player_transport_control_tests
    player_qml_lint
)

target_compile_definitions(
    player_transport_control_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_transport_control_tests
    PRIVATE
        Qt6::Test
)

add_test(
    NAME player_transport_controls
    COMMAND player_transport_control_tests
)
