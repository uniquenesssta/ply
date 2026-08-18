add_executable(
    track_selection_controller_tests
    track_selection_controller_test.cpp
)

player_apply_compiler_options(track_selection_controller_tests)
player_enable_compiler_warnings(track_selection_controller_tests)
player_enable_sanitizers(track_selection_controller_tests)
player_enable_static_analysis(track_selection_controller_tests)

target_link_libraries(
    track_selection_controller_tests
    PRIVATE
        player_tracks_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME track_selection_controller
    COMMAND track_selection_controller_tests
)
