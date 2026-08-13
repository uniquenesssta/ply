add_executable(feedback_control_tests feedback_control_test.cpp)

player_apply_compiler_options(feedback_control_tests)
player_enable_compiler_warnings(feedback_control_tests)
player_enable_sanitizers(feedback_control_tests)
player_enable_static_analysis(feedback_control_tests)

add_dependencies(feedback_control_tests player_qml_lint)

target_link_libraries(
    feedback_control_tests
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
        player_presentation_controls
        player_presentation_surfaces
        player_presentation_feedback
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(NAME feedback_controls COMMAND feedback_control_tests)
set_tests_properties(feedback_controls PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
