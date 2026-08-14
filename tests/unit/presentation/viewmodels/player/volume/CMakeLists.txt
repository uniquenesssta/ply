add_executable(
    player_volume_view_model_tests
    player_volume_view_model_test.cpp
)

player_apply_compiler_options(player_volume_view_model_tests)
player_enable_compiler_warnings(player_volume_view_model_tests)
player_enable_sanitizers(player_volume_view_model_tests)
player_enable_static_analysis(player_volume_view_model_tests)

target_link_libraries(
    player_volume_view_model_tests
    PRIVATE
        player_presentation_volume
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_volume_view_model
    COMMAND player_volume_view_model_tests
)
