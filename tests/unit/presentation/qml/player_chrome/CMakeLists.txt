add_executable(
    player_chrome_visibility_tests
    player_chrome_visibility_test.cpp
)

player_apply_compiler_options(player_chrome_visibility_tests)
player_enable_compiler_warnings(player_chrome_visibility_tests)
player_enable_sanitizers(player_chrome_visibility_tests)
player_enable_static_analysis(player_chrome_visibility_tests)

add_dependencies(
    player_chrome_visibility_tests
    player_qml_lint
)

target_compile_definitions(
    player_chrome_visibility_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_chrome_visibility_tests
    PRIVATE
        player_presentation_theme
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME player_chrome_visibility
    COMMAND player_chrome_visibility_tests
)

set_tests_properties(
    player_chrome_visibility
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    player_controls_visibility_policy_tests
    controls_visibility_policy_test.cpp
)

player_apply_compiler_options(player_controls_visibility_policy_tests)
player_enable_compiler_warnings(player_controls_visibility_policy_tests)
player_enable_sanitizers(player_controls_visibility_policy_tests)
player_enable_static_analysis(player_controls_visibility_policy_tests)

add_dependencies(
    player_controls_visibility_policy_tests
    player_qml_lint
)

target_compile_definitions(
    player_controls_visibility_policy_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_controls_visibility_policy_tests
    PRIVATE
        player_presentation_theme
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME player_controls_visibility_policy
    COMMAND player_controls_visibility_policy_tests
)

set_tests_properties(
    player_controls_visibility_policy
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    player_cursor_visibility_tests
    player_cursor_visibility_test.cpp
)

player_apply_compiler_options(player_cursor_visibility_tests)
player_enable_compiler_warnings(player_cursor_visibility_tests)
player_enable_sanitizers(player_cursor_visibility_tests)
player_enable_static_analysis(player_cursor_visibility_tests)

add_dependencies(
    player_cursor_visibility_tests
    player_qml_lint
)

target_compile_definitions(
    player_cursor_visibility_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_cursor_visibility_tests
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME player_cursor_visibility
    COMMAND player_cursor_visibility_tests
)

set_tests_properties(
    player_cursor_visibility
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    player_cursor_visibility_policy_tests
    cursor_visibility_policy_test.cpp
)

player_apply_compiler_options(player_cursor_visibility_policy_tests)
player_enable_compiler_warnings(player_cursor_visibility_policy_tests)
player_enable_sanitizers(player_cursor_visibility_policy_tests)
player_enable_static_analysis(player_cursor_visibility_policy_tests)

add_dependencies(
    player_cursor_visibility_policy_tests
    player_qml_lint
)

target_compile_definitions(
    player_cursor_visibility_policy_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_cursor_visibility_policy_tests
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME player_cursor_visibility_policy
    COMMAND player_cursor_visibility_policy_tests
)

set_tests_properties(
    player_cursor_visibility_policy
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
