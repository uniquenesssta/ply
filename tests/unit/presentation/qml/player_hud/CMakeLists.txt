add_executable(
    player_hud_overlay_tests
    player_hud_overlay_test.cpp
)

player_apply_compiler_options(player_hud_overlay_tests)
player_enable_compiler_warnings(player_hud_overlay_tests)
player_enable_sanitizers(player_hud_overlay_tests)
player_enable_static_analysis(player_hud_overlay_tests)

add_dependencies(
    player_hud_overlay_tests
    player_qml_lint
)

target_compile_definitions(
    player_hud_overlay_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_hud_overlay_tests
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
        player_presentation_surfaces
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME player_hud_overlay
    COMMAND player_hud_overlay_tests
)

set_tests_properties(
    player_hud_overlay
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
