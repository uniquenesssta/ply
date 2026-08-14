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
