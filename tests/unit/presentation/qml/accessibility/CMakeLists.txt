add_executable(
    accessibility_control_tests
    accessibility_control_test.cpp
)

player_apply_compiler_options(accessibility_control_tests)
player_enable_compiler_warnings(accessibility_control_tests)
player_enable_sanitizers(accessibility_control_tests)
player_enable_static_analysis(accessibility_control_tests)

add_dependencies(
    accessibility_control_tests
    player_qml_lint
)

target_link_libraries(
    accessibility_control_tests
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
    NAME accessibility_controls
    COMMAND accessibility_control_tests
)

set_tests_properties(
    accessibility_controls
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
