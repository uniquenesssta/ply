add_executable(
    player_track_selection_qml_tests
    track_selection_qml_test.cpp
)

player_apply_compiler_options(player_track_selection_qml_tests)
player_enable_compiler_warnings(player_track_selection_qml_tests)
player_enable_sanitizers(player_track_selection_qml_tests)
player_enable_static_analysis(player_track_selection_qml_tests)

add_dependencies(
    player_track_selection_qml_tests
    player_qml_lint
)

target_compile_definitions(
    player_track_selection_qml_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_track_selection_qml_tests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_track_selection_qml
    COMMAND player_track_selection_qml_tests
)
