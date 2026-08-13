add_executable(
    qml_module_boundary_tests
    qml_module_boundary_test.cpp
)

player_apply_compiler_options(qml_module_boundary_tests)
player_enable_compiler_warnings(qml_module_boundary_tests)
player_enable_sanitizers(qml_module_boundary_tests)
player_enable_static_analysis(qml_module_boundary_tests)

add_dependencies(
    qml_module_boundary_tests
    player_qml_lint
)

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

add_executable(
    theme_token_tests
    theme_token_test.cpp
)

player_apply_compiler_options(theme_token_tests)
player_enable_compiler_warnings(theme_token_tests)
player_enable_sanitizers(theme_token_tests)
player_enable_static_analysis(theme_token_tests)

add_dependencies(
    theme_token_tests
    player_qml_lint
)

target_compile_definitions(
    theme_token_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    theme_token_tests
    PRIVATE
        player_presentation_theme
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME theme_tokens
    COMMAND theme_token_tests
)

set_tests_properties(
    theme_tokens
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
