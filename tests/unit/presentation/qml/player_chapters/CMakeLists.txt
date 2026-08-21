add_executable(
    player_chapters_qml_tests
    chapters_qml_test.cpp
)

player_apply_compiler_options(player_chapters_qml_tests)
player_enable_compiler_warnings(player_chapters_qml_tests)
player_enable_sanitizers(player_chapters_qml_tests)
player_enable_static_analysis(player_chapters_qml_tests)

add_dependencies(
    player_chapters_qml_tests
    player_qml_lint
)

target_compile_definitions(
    player_chapters_qml_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_chapters_qml_tests
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
        player_presentation_controls
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME player_chapters_qml
    COMMAND player_chapters_qml_tests
)

set_tests_properties(
    player_chapters_qml
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
