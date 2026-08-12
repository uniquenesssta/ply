add_executable(
    qml_module_boundary_tests
    qml_module_boundary_test.cpp
)

player_apply_compiler_options(qml_module_boundary_tests)
player_enable_compiler_warnings(qml_module_boundary_tests)
player_enable_sanitizers(qml_module_boundary_tests)
player_enable_static_analysis(qml_module_boundary_tests)

target_compile_definitions(
    qml_module_boundary_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    qml_module_boundary_tests
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
        player_presentation_controls
        player_presentation_surfaces
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME qml_module_boundaries
    COMMAND qml_module_boundary_tests
)

set_tests_properties(
    qml_module_boundaries
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
