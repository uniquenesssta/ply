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

add_executable(
    theme_effect_token_tests
    theme_effect_token_test.cpp
)

player_apply_compiler_options(theme_effect_token_tests)
player_enable_compiler_warnings(theme_effect_token_tests)
player_enable_sanitizers(theme_effect_token_tests)
player_enable_static_analysis(theme_effect_token_tests)

add_dependencies(
    theme_effect_token_tests
    player_qml_lint
)

target_compile_definitions(
    theme_effect_token_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    theme_effect_token_tests
    PRIVATE
        player_presentation_theme
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME theme_effect_tokens
    COMMAND theme_effect_token_tests
)

set_tests_properties(
    theme_effect_tokens
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    icon_pipeline_tests
    icon_pipeline_test.cpp
)

player_apply_compiler_options(icon_pipeline_tests)
player_enable_compiler_warnings(icon_pipeline_tests)
player_enable_sanitizers(icon_pipeline_tests)
player_enable_static_analysis(icon_pipeline_tests)

add_dependencies(
    icon_pipeline_tests
    player_qml_lint
)

target_compile_definitions(
    icon_pipeline_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    icon_pipeline_tests
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME icon_pipeline
    COMMAND icon_pipeline_tests
)

set_tests_properties(
    icon_pipeline
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    typography_primitive_tests
    typography_primitive_test.cpp
)

player_apply_compiler_options(typography_primitive_tests)
player_enable_compiler_warnings(typography_primitive_tests)
player_enable_sanitizers(typography_primitive_tests)
player_enable_static_analysis(typography_primitive_tests)

add_dependencies(
    typography_primitive_tests
    player_qml_lint
)

target_compile_definitions(
    typography_primitive_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    typography_primitive_tests
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME typography_primitives
    COMMAND typography_primitive_tests
)

set_tests_properties(
    typography_primitives
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    button_control_tests
    button_control_test.cpp
)

player_apply_compiler_options(button_control_tests)
player_enable_compiler_warnings(button_control_tests)
player_enable_sanitizers(button_control_tests)
player_enable_static_analysis(button_control_tests)

add_dependencies(
    button_control_tests
    player_qml_lint
)

target_link_libraries(
    button_control_tests
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
    NAME button_controls
    COMMAND button_control_tests
)

set_tests_properties(
    button_controls
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    slider_control_tests
    slider_control_test.cpp
)

player_apply_compiler_options(slider_control_tests)
player_enable_compiler_warnings(slider_control_tests)
player_enable_sanitizers(slider_control_tests)
player_enable_static_analysis(slider_control_tests)

add_dependencies(
    slider_control_tests
    player_qml_lint
)

target_link_libraries(
    slider_control_tests
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
    NAME slider_controls
    COMMAND slider_control_tests
)

set_tests_properties(
    slider_controls
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    surface_control_tests
    surface_control_test.cpp
)

player_apply_compiler_options(surface_control_tests)
player_enable_compiler_warnings(surface_control_tests)
player_enable_sanitizers(surface_control_tests)
player_enable_static_analysis(surface_control_tests)

add_dependencies(
    surface_control_tests
    player_qml_lint
)

target_compile_definitions(
    surface_control_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    surface_control_tests
    PRIVATE
        player_presentation_theme
        player_presentation_surfaces
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME surface_controls
    COMMAND surface_control_tests
)

set_tests_properties(
    surface_controls
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_subdirectory(accessibility)
add_subdirectory(feedback)
add_subdirectory(player_screen)
add_subdirectory(player_transport)
add_subdirectory(player_timeline)
